#include "services.h"
#include <QMessageAuthenticationCode>
#include <QDateTime>
#include <QFileInfo>
#include <QPrinter>
#include <QTextDocument>
#include <QRegularExpression>
#include <QSqlQuery>
#include <QUuid>
#include <algorithm>

namespace {
const QTime bookingDayStarts(8,30);
const QTime bookingDayEnds(17,30);
bool validPerson(const UserRecord&user){
  if(user.eid.trimmed().isEmpty()||user.eid.trimmed().size()>32)return false;
  if(user.name.trimmed().isEmpty()||user.name.trimmed().size()>120)return false;
  if(user.email.trimmed().size()>160)return false;
  return user.email.trimmed().isEmpty()||QRegularExpression("^[^\\s@]+@[^\\s@]+\\.[^\\s@]+$").match(user.email.trimmed()).hasMatch();
}
QString escapeHtml(QString value){
  return value.toHtmlEscaped();
}
}

const QVector<FixedBookingSlot>& fixedBookingSlots() {
  static const QVector<FixedBookingSlot> slotList = {
    {1, QTime(8, 30), QTime(10, 30), "8:30 AM – 10:30 AM", "Slot 1 (8:30 AM – 10:30 AM)"},
    {2, QTime(10, 30), QTime(12, 30), "10:30 AM – 12:30 PM", "Slot 2 (10:30 AM – 12:30 PM)"},
    {3, QTime(13, 30), QTime(15, 30), "1:30 PM – 3:30 PM", "Slot 3 (1:30 PM – 3:30 PM)"},
    {4, QTime(15, 30), QTime(17, 30), "3:30 PM – 5:30 PM", "Slot 4 (3:30 PM – 5:30 PM)"}
  };
  return slotList;
}

std::optional<FixedBookingSlot> findFixedBookingSlot(const QTime& start, const QTime& end) {
  for (const auto& slot : fixedBookingSlots()) {
    if (slot.start == start && slot.end == end) return slot;
  }
  return std::nullopt;
}

bool isFixedBookingSlot(const QTime& start, const QTime& end) {
  return findFixedBookingSlot(start, end).has_value();
}

bool isWeekend(const QDate& date) {
  return date.isValid() && (date.dayOfWeek() == Qt::Saturday || date.dayOfWeek() == Qt::Sunday);
}

bool overlapsLunchBreak(const QTime& start, const QTime& end) {
  const QTime lunchStart(12, 30);
  const QTime lunchEnd(13, 30);
  return start < lunchEnd && end > lunchStart;
}

QDate nextOperatingWeekday(const QDate& from) {
  QDate d = from.isValid() ? from.addDays(1) : QDate::currentDate().addDays(1);
  while (d.dayOfWeek() == Qt::Saturday || d.dayOfWeek() == Qt::Sunday) {
    d = d.addDays(1);
  }
  return d;
}

bool hasPermission(const User&user,Permission permission){
  const auto permissions=user.permissions();
  return permissions.contains(permission);
}

QString toString(UserRole role){
  switch(role){case UserRole::Administrator:return "Administrator";case UserRole::Lecturer:return "Lecturer";default:return "Student";}
}
QString toString(LaboratoryStatus status){
  switch(status){case LaboratoryStatus::Unavailable:return "Unavailable";case LaboratoryStatus::UnderMaintenance:return "Under Maintenance";default:return "Available";}
}
QString toString(EquipmentStatus status){
  switch(status){case EquipmentStatus::Unavailable:return "Unavailable";case EquipmentStatus::Maintenance:return "Maintenance";default:return "Available";}
}
QString toString(BookingStatus status){
  switch(status){case BookingStatus::Approved:return "Approved";case BookingStatus::Rejected:return "Rejected";case BookingStatus::Cancelled:return "Cancelled";case BookingStatus::Completed:return "Completed";default:return "Pending";}
}
UserRole userRoleFromString(const QString&value){
  if(value.compare("Administrator",Qt::CaseInsensitive)==0)return UserRole::Administrator;
  if(value.compare("Lecturer",Qt::CaseInsensitive)==0)return UserRole::Lecturer;
  return UserRole::Student;
}
BookingStatus bookingStatusFromString(const QString&value){
  if(value=="Approved")return BookingStatus::Approved;
  if(value=="Rejected")return BookingStatus::Rejected;
  if(value=="Cancelled")return BookingStatus::Cancelled;
  if(value=="Completed")return BookingStatus::Completed;
  return BookingStatus::Pending;
}
User::User(qint64 id,QString eid,QString name,UserRole role,bool active,bool mustChange)
  :m_id(id),m_eid(std::move(eid)),m_name(std::move(name)),m_role(role),m_active(active),m_mustChange(mustChange){}
qint64 User::id()const{return m_id;}
QString User::eid()const{return m_eid;}
QString User::name()const{return m_name;}
UserRole User::role()const{return m_role;}
bool User::active()const{return m_active;}
bool User::mustChangePassword()const{return m_mustChange;}
QString Student::roleName()const{return "Student";}
QString Lecturer::roleName()const{return "Lecturer";}
QString Administrator::roleName()const{return "Administrator";}
QVector<Permission> Student::permissions()const{return {Permission::ViewDashboard,Permission::CreateBooking,Permission::ManageOwnBooking};}
QVector<Permission> Lecturer::permissions()const{return {Permission::ViewDashboard,Permission::CreateBooking,Permission::ManageOwnBooking};}
QVector<Permission> Administrator::permissions()const{return {Permission::ViewDashboard,Permission::CreateBooking,Permission::ManageOwnBooking,Permission::ManageUsers,Permission::ManageLabs,Permission::ManageEquipment,Permission::ReviewBookings,Permission::ViewReports};}

QString AuthService::salt(){
  QByteArray bytes(32,Qt::Uninitialized);
  auto*random=QRandomGenerator::system();
  for(int i=0;i<bytes.size();i+=4){
    const quint32 value=random->generate();
    for(int j=0;j<4&&i+j<bytes.size();++j)bytes[i+j]=char((value>>(j*8))&0xff);
  }
  return QString::fromLatin1(bytes.toBase64());
}
QString AuthService::derive(const QString&password,const QString&encodedSalt,int iterations){
  const QByteArray key=password.toUtf8();
  QByteArray block=QByteArray::fromBase64(encodedSalt.toUtf8());
  block.append(char(0));block.append(char(0));block.append(char(0));block.append(char(1));
  QByteArray u=QMessageAuthenticationCode::hash(block,key,QCryptographicHash::Sha256);
  QByteArray result=u;
  for(int i=1;i<iterations;++i){
    u=QMessageAuthenticationCode::hash(u,key,QCryptographicHash::Sha256);
    for(int j=0;j<result.size();++j)result[j]=char(uchar(result[j])^uchar(u[j]));
  }
  return QString::fromLatin1(result.toBase64());
}
bool AuthService::verify(const QString&password,const QString&expected,const QString&saltValue,int iterations){
  const QByteArray a=derive(password,saltValue,iterations).toUtf8();
  const QByteArray b=expected.toUtf8();
  if(a.size()!=b.size())return false;
  uchar difference=0;
  for(qsizetype i=0;i<a.size();++i)difference|=uchar(a.at(i))^uchar(b.at(i));
  return difference==0;
}
ServiceResult AuthService::bootstrap(const UserRecord&user,const QString&password){
  if(!validPerson(user))return ServiceResult::failure("Enter a valid E-ID, name, and email address.");
  if(password.size()<8)return ServiceResult::failure("Password must be at least 8 characters.");
  if(user.role!=UserRole::Administrator)return ServiceResult::failure("The first account must be an administrator.");
  if(m_db.hasUsers())return ServiceResult::failure("Initial administrator already exists.");
  const auto saltValue=salt();
  return m_db.transaction([&]{return m_db.insertUser(user,derive(password,saltValue,210000),saltValue,210000,false);});
}
ServiceResult AuthService::createUser(const UserRecord&user,const QString&password){
  if(!validPerson(user))return ServiceResult::failure("Enter a valid E-ID, name, and email address.");
  if(password.size()<8)return ServiceResult::failure("Temporary password must be at least 8 characters.");
  const auto saltValue=salt();
  return m_db.transaction([&]{return m_db.insertUser(user,derive(password,saltValue,210000),saltValue,210000,true);});
}
ServiceResult AuthService::registerAccount(const UserRecord&user,const QString&password){
  if(!validPerson(user)||user.email.trimmed().isEmpty())return ServiceResult::failure("Enter a valid university E-ID, name, and email address.");
  if(user.role!=UserRole::Student)return ServiceResult::failure("Self-registration is available for student accounts only.");
  if(password.size()<8)return ServiceResult::failure("Password must be at least 8 characters.");
  const auto saltValue=salt();
  return m_db.transaction([&]{return m_db.insertUser(user,derive(password,saltValue,210000),saltValue,210000,false,false);});
}
std::shared_ptr<User> AuthService::authenticate(const QString&eid,const QString&password,QString*error)const{
  const auto record=m_db.userByEid(eid);
  if(!record||!record->active){
    if(error)*error="Invalid E-ID or password.";
    return {};
  }
  QSqlQuery q(m_db.db());
  q.prepare("SELECT password_hash,password_salt,password_iterations FROM users WHERE user_id=?");
  q.addBindValue(record->id);
  if(!q.exec()||!q.next()||!verify(password,q.value(0).toString(),q.value(1).toString(),q.value(2).toInt())){
    if(error)*error="Invalid E-ID or password.";
    return {};
  }
  if(!record->approved){if(error)*error="Your account request is waiting for administrator approval.";return {};}
  if(error)error->clear();
  switch(record->role){
    case UserRole::Administrator:return std::make_shared<Administrator>(record->id,record->eid,record->name,record->role,record->active,record->mustChange);
    case UserRole::Lecturer:return std::make_shared<Lecturer>(record->id,record->eid,record->name,record->role,record->active,record->mustChange);
    default:return std::make_shared<Student>(record->id,record->eid,record->name,record->role,record->active,record->mustChange);
  }
}
ServiceResult AuthService::changePassword(qint64 id,const QString&oldPassword,const QString&newPassword){
  if(newPassword.size()<8)return ServiceResult::failure("Password must be at least 8 characters.");
  QSqlQuery q(m_db.db());q.prepare("SELECT password_hash,password_salt,password_iterations FROM users WHERE user_id=?");q.addBindValue(id);
  if(!q.exec()||!q.next()||!verify(oldPassword,q.value(0).toString(),q.value(1).toString(),q.value(2).toInt()))
    return ServiceResult::failure("Current password is incorrect.");
  const auto saltValue=salt();
  return m_db.setPassword(id,derive(newPassword,saltValue,210000),saltValue,210000,false);
}

ServiceResult UserManager::add(const User&actor,const UserRecord&record,const QString&temporaryPassword){
  if(!hasPermission(actor,Permission::ManageUsers))return ServiceResult::failure("You do not have permission to manage users.");
  return AuthService(m_db).createUser(record,temporaryPassword);
}
ServiceResult UserManager::update(const User&actor,const UserRecord&record){
  if(!hasPermission(actor,Permission::ManageUsers))return ServiceResult::failure("You do not have permission to manage users.");
  if(!validPerson(record))return ServiceResult::failure("Enter a valid E-ID, name, and email address.");
  const auto existing=m_db.userById(record.id);
  if(!existing)return ServiceResult::failure("User was not found.");
  if(existing->role==UserRole::Administrator&&record.role!=UserRole::Administrator&&existing->active&&m_db.count("users","active=1 AND role_id=3")<=1)
    return ServiceResult::failure("The last active administrator must remain an administrator.");
  return m_db.updateUser(record);
}
ServiceResult UserManager::approve(const User&actor,qint64 id){
  if(!hasPermission(actor,Permission::ManageUsers))return ServiceResult::failure("Only an administrator can approve account requests.");
  const auto user=m_db.userById(id);
  if(!user)return ServiceResult::failure("User was not found.");
  if(user->approved)return ServiceResult::failure("This account has already been approved.");
  return m_db.transaction([&]{ return m_db.setUserApproved(id,true); });
}
ServiceResult UserManager::deactivate(const User&actor,qint64 id){
  if(!hasPermission(actor,Permission::ManageUsers))return ServiceResult::failure("You do not have permission to manage users.");
  const auto user=m_db.userById(id);
  if(!user)return ServiceResult::failure("User was not found.");
  if(user->id==actor.id())return ServiceResult::failure("You cannot deactivate your own account.");
  if(user->active&&user->role==UserRole::Administrator&&m_db.count("users","active=1 AND role_id=3")<=1)
    return ServiceResult::failure("The last active administrator cannot be deactivated.");
  return m_db.setUserActive(id,false);
}
ServiceResult UserManager::activate(const User&actor,qint64 id){
  if(!hasPermission(actor,Permission::ManageUsers))return ServiceResult::failure("You do not have permission to manage users.");
  const auto user=m_db.userById(id);
  if(!user)return ServiceResult::failure("User was not found.");
  return m_db.setUserActive(id,true);
}
QVector<UserRecord> UserManager::search(const User&actor,const QString&query)const{
  if(!hasPermission(actor,Permission::ManageUsers))return {};
  return m_db.users(query,true);
}
ServiceResult LaboratoryManager::add(const User&actor,const Laboratory&lab){
  if(!hasPermission(actor,Permission::ManageLabs))return ServiceResult::failure("You do not have permission to manage laboratories.");
  if(lab.name.trimmed().isEmpty()||lab.number.trimmed().isEmpty()||lab.capacity<=0)return ServiceResult::failure("Name, number, and a positive capacity are required.");
  return m_db.addLaboratory(lab);
}
ServiceResult LaboratoryManager::update(const User&actor,const Laboratory&lab){
  if(!hasPermission(actor,Permission::ManageLabs))return ServiceResult::failure("You do not have permission to manage laboratories.");
  if(lab.name.trimmed().isEmpty()||lab.number.trimmed().isEmpty()||lab.capacity<=0)return ServiceResult::failure("Name, number, and a positive capacity are required.");
  return m_db.updateLaboratory(lab);
}
ServiceResult LaboratoryManager::deactivate(const User&actor,qint64 id){
  if(!hasPermission(actor,Permission::ManageLabs))return ServiceResult::failure("You do not have permission to manage laboratories.");
  const auto today=QDate::currentDate().toString(Qt::ISODate);
  if(m_db.count("bookings","lab_id="+QString::number(id)+" AND booking_date>='"+today+"' AND status IN ('Pending','Approved')")>0)
    return ServiceResult::failure("This laboratory has future bookings. Cancel or complete them before deactivating it.");
  return m_db.setLaboratoryActive(id,false);
}
ServiceResult LaboratoryManager::activate(const User&actor,qint64 id){
  if(!hasPermission(actor,Permission::ManageLabs))return ServiceResult::failure("You do not have permission to manage laboratories.");
  return m_db.setLaboratoryActive(id,true);
}
QVector<Laboratory> LaboratoryManager::search(const User&actor,const QString&query)const{
  if(!hasPermission(actor,Permission::ManageLabs))return {};
  QVector<Laboratory> matches;
  for(const auto&lab:m_db.laboratories(true)){
    const QString haystack=lab.name+" "+lab.number+" "+lab.location+" "+lab.department;
    if(query.trimmed().isEmpty()||haystack.contains(query.trimmed(),Qt::CaseInsensitive))matches.push_back(lab);
  }
  return matches;
}

bool AvailabilityService::check(qint64 lab,const QDate&date,const QTime&start,const QTime&end,qint64 exclude)const{
  return validateRequest(lab,date,start,end,exclude).ok;
}
ServiceResult AvailabilityService::validateRequest(qint64 lab,const QDate&date,const QTime&start,const QTime&end,qint64 exclude)const{
  if(lab<=0)return ServiceResult::failure("Select a laboratory.");
  if(!date.isValid())return ServiceResult::failure("Select a valid date.");
  if(isWeekend(date))return ServiceResult::failure("Weekend bookings are not allowed.");
  if(m_db.isClosedDate(date))return ServiceResult::failure("The laboratory is closed on this date.");
  if(!start.isValid()||!end.isValid()||start>=end)return ServiceResult::failure("End time must be after start time.");
  const auto configuredSlots = m_db.bookingSlots();
  if (start < configuredSlots.first().start || end > configuredSlots.last().end)
    return ServiceResult::failure("This time is outside the configured operating hours.");
  if (start < configuredSlots[2].start && end > configuredSlots[1].end)
    return ServiceResult::failure("Bookings cannot overlap the lunch break.");
  bool validSlot = false;
  for (const auto& slot : configuredSlots) if (slot.start == start && slot.end == end) validSlot = true;
  if(!validSlot)return ServiceResult::failure("Choose one of the four configured 2 hours sessions.");
  if(date<QDate::currentDate()||(date==QDate::currentDate()&&start<=QTime::currentTime()))return ServiceResult::failure("Choose a future date and time.");
  if(!m_db.isAvailable(lab,date,start,end,exclude))return ServiceResult::failure("Laboratory is already reserved or unavailable for this slot.");
  return {};
}
QVector<Laboratory> AvailabilityService::available(const QDate&date,const QTime&start,const QTime&end)const{
  QVector<Laboratory> result;
  if(!date.isValid()||m_db.isClosedDate(date))return result;
  for(const auto&lab:m_db.laboratories())if(check(lab.id,date,start,end))result.push_back(lab);
  return result;
}

ServiceResult BookingService::create(const User&actor,BookingRequest request,QString*reference){
  if(!hasPermission(actor,Permission::CreateBooking))return ServiceResult::failure("You do not have permission to create bookings.");
  if(!actor.active())return ServiceResult::failure("Inactive users cannot create bookings.");
  request.userId=actor.id();
  if(request.purpose.trimmed().isEmpty())return ServiceResult::failure("Enter a purpose for the booking.");
  const auto timeCheck=m_availability.validateRequest(request.labId,request.date,request.start,request.end);
  if(!timeCheck.ok)return timeCheck;
  return m_db.transaction([&]{
    if(!m_db.isAvailable(request.labId,request.date,request.start,request.end))return ServiceResult::failure("Booking conflict. The laboratory is already reserved during the selected period.");
    auto equipmentCheck=m_db.checkEquipmentAvailability(request.labId,request.date,request.start,request.end,request.equipment);
    if(!equipmentCheck.ok)return equipmentCheck;
    const QString id=QUuid::createUuid().toString(QUuid::WithoutBraces).left(8).toUpper();
    const QString ref="LB-"+id;
    auto result=m_db.insertBooking(request,ref);
    if(result.ok&&reference)*reference=ref;
    return result;
  });
}
ServiceResult BookingService::update(const User&actor,qint64 id,BookingRequest request){
  if(!hasPermission(actor,Permission::ManageOwnBooking))return ServiceResult::failure("You do not have permission to change bookings.");
  const auto current=m_db.bookingById(id);
  if(!current||current->userId!=actor.id())return ServiceResult::failure("You may only change your own bookings.");
  if(current->status!=BookingStatus::Pending)return ServiceResult::failure("Only pending bookings may be changed.");
  request.userId=actor.id();
  if(request.purpose.trimmed().isEmpty())return ServiceResult::failure("Enter a purpose for the booking.");
  const auto timeCheck=m_availability.validateRequest(request.labId,request.date,request.start,request.end,id);
  if(!timeCheck.ok)return timeCheck;
  return m_db.transaction([&]{
    if(!m_db.isAvailable(request.labId,request.date,request.start,request.end,id))return ServiceResult::failure("Booking conflict. The laboratory is already reserved during the selected period.");
    const auto equipmentCheck=m_db.checkEquipmentAvailability(request.labId,request.date,request.start,request.end,request.equipment,id);
    if(!equipmentCheck.ok)return equipmentCheck;
    return m_db.updateBooking(*current,request);
  });
}
ServiceResult BookingService::cancel(const User&actor,qint64 id,const QString&note){
  const auto booking=m_db.bookingById(id);
  if(!booking)return ServiceResult::failure("Booking was not found.");
  const bool admin=hasPermission(actor,Permission::ReviewBookings);
  if(!admin&&!hasPermission(actor,Permission::ManageOwnBooking))return ServiceResult::failure("You do not have permission to cancel bookings.");
  if(!admin&&booking->userId!=actor.id())return ServiceResult::failure("You may only cancel your own bookings.");
  if(!admin&&(booking->status==BookingStatus::Pending||booking->status==BookingStatus::Approved)
     &&(booking->date<QDate::currentDate()||(booking->date==QDate::currentDate()&&booking->start<=QTime::currentTime())))
    return ServiceResult::failure("Bookings that have already started cannot be cancelled by the requester.");
  if(booking->status!=BookingStatus::Pending&&booking->status!=BookingStatus::Approved)return ServiceResult::failure("Only pending or approved bookings may be cancelled.");
  return m_db.transaction([&]{return m_db.cancelBooking(id,actor.id(),note);});
}
ServiceResult BookingService::review(const User&actor,qint64 id,BookingStatus status,const QString&note){
  if(!hasPermission(actor,Permission::ReviewBookings))return ServiceResult::failure("Only laboratory administrators can review bookings.");
  const auto booking=m_db.bookingById(id);
  if(!booking)return ServiceResult::failure("Booking was not found.");
  if(status==BookingStatus::Approved){
    if(booking->status!=BookingStatus::Pending)return ServiceResult::failure("Only pending bookings can be approved.");
    if(!m_db.isAvailable(booking->labId,booking->date,booking->start,booking->end,id))return ServiceResult::failure("The laboratory is no longer available for this booking.");
  }else if(status==BookingStatus::Rejected){
    if(booking->status!=BookingStatus::Pending)return ServiceResult::failure("Only pending bookings can be rejected.");
  }else if(status==BookingStatus::Completed){
    if(booking->status!=BookingStatus::Approved||booking->date>QDate::currentDate()||(booking->date==QDate::currentDate()&&booking->end>QTime::currentTime()))
      return ServiceResult::failure("Only an approved booking that has ended can be completed.");
  }else if(status==BookingStatus::Cancelled){
    if(booking->status!=BookingStatus::Pending&&booking->status!=BookingStatus::Approved)return ServiceResult::failure("Only pending or approved bookings can be cancelled.");
  }else return ServiceResult::failure("That booking action is not allowed.");
  return m_db.transaction([&]{return m_db.setBookingStatus(id,status,actor.id(),note);});
}

ServiceResult EquipmentManager::add(const User&actor,const Equipment&equipment){
  if(!hasPermission(actor,Permission::ManageEquipment))return ServiceResult::failure("You do not have permission to manage equipment.");
  if(equipment.labId<=0||equipment.name.trimmed().isEmpty())return ServiceResult::failure("Select a laboratory and enter an equipment name.");
  if(equipment.totalQty<0||equipment.availableQty<0||equipment.availableQty>equipment.totalQty)return ServiceResult::failure("Equipment quantities must satisfy 0 <= available <= total.");
  return m_db.addEquipment(equipment);
}
ServiceResult EquipmentManager::update(const User&actor,const Equipment&equipment){
  if(!hasPermission(actor,Permission::ManageEquipment))return ServiceResult::failure("You do not have permission to manage equipment.");
  if(equipment.labId<=0||equipment.name.trimmed().isEmpty())return ServiceResult::failure("Select a laboratory and enter an equipment name.");
  if(equipment.totalQty<0||equipment.availableQty<0||equipment.availableQty>equipment.totalQty)return ServiceResult::failure("Equipment quantities must satisfy 0 <= available <= total.");
  return m_db.updateEquipment(equipment);
}
ServiceResult EquipmentManager::deactivate(const User&actor,qint64 id){
  if(!hasPermission(actor,Permission::ManageEquipment))return ServiceResult::failure("You do not have permission to manage equipment.");
  return m_db.setEquipmentActive(id,false);
}
ServiceResult EquipmentManager::activate(const User&actor,qint64 id){
  if(!hasPermission(actor,Permission::ManageEquipment))return ServiceResult::failure("You do not have permission to manage equipment.");
  return m_db.setEquipmentActive(id,true);
}
QVector<Equipment> EquipmentManager::search(const User&actor,const QString&query)const{
  if(!hasPermission(actor,Permission::ManageEquipment))return {};
  QVector<Equipment> matches;
  for(const auto&item:m_db.equipment(true))if(query.trimmed().isEmpty()||item.name.contains(query.trimmed(),Qt::CaseInsensitive))matches.push_back(item);
  return matches;
}

ReportCounts ReportService::bookingCounts(const QDate&from,const QDate&to)const{
  ReportCounts counts;
  QSqlQuery q(m_db.db());
  q.prepare("SELECT status,COUNT(*) FROM bookings WHERE (? IS NULL OR booking_date>=?) AND (? IS NULL OR booking_date<=?) GROUP BY status");
  const QVariant fromValue=from.isValid()?QVariant(from.toString(Qt::ISODate)):QVariant();
  const QVariant toValue=to.isValid()?QVariant(to.toString(Qt::ISODate)):QVariant();
  q.addBindValue(fromValue);q.addBindValue(fromValue);q.addBindValue(toValue);q.addBindValue(toValue);
  if(!q.exec())return counts;
  while(q.next()){
    const int value=q.value(1).toInt();counts.total+=value;
    const QString status=q.value(0).toString();
    if(status=="Pending")counts.pending=value;else if(status=="Approved")counts.approved=value;
    else if(status=="Rejected")counts.rejected=value;else if(status=="Cancelled")counts.cancelled=value;
    else if(status=="Completed")counts.completed=value;
  }
  return counts;
}
QHash<QString,int> ReportService::laboratoryCounts()const{
  QHash<QString,int> out;
  out.insert("Total",m_db.count("laboratories","active=1"));
  out.insert("Available",m_db.count("laboratories","active=1 AND status='Available'"));
  out.insert("Unavailable",m_db.count("laboratories","active=1 AND status='Unavailable'"));
  out.insert("Under Maintenance",m_db.count("laboratories","active=1 AND status='Under Maintenance'"));
  return out;
}
QHash<QString,int> ReportService::equipmentCounts()const{
  QHash<QString,int> out;
  out.insert("Available",m_db.count("equipment","active=1 AND status='Available'"));
  out.insert("Unavailable",m_db.count("equipment","active=1 AND status='Unavailable'"));
  out.insert("Maintenance",m_db.count("equipment","active=1 AND status='Maintenance'"));
  return out;
}
QString ReportService::summaryHtml(const QDate&from,const QDate&to)const{
  const auto bookings=bookingCounts(from,to);
  const auto labs=laboratoryCounts();
  const auto gear=equipmentCounts();
  return QString("<h1>LabBook report</h1><p>Generated %1 UTC</p><p>Booking period: %2 to %3</p>"
    "<h2>Laboratories</h2><table border='1' cellspacing='0' cellpadding='6'><tr><th>Total</th><th>Available</th><th>Unavailable</th><th>Maintenance</th></tr><tr><td>%4</td><td>%5</td><td>%6</td><td>%7</td></tr></table>"
    "<h2>Bookings</h2><table border='1' cellspacing='0' cellpadding='6'><tr><th>Total</th><th>Pending</th><th>Approved</th><th>Rejected</th><th>Cancelled</th><th>Completed</th></tr><tr><td>%8</td><td>%9</td><td>%10</td><td>%11</td><td>%12</td><td>%13</td></tr></table>"
    "<h2>Equipment</h2><table border='1' cellspacing='0' cellpadding='6'><tr><th>Available</th><th>Unavailable</th><th>Maintenance</th></tr><tr><td>%14</td><td>%15</td><td>%16</td></tr></table>")
    .arg(QDateTime::currentDateTimeUtc().toString(Qt::ISODate))
    .arg(from.isValid()?from.toString(Qt::ISODate):"All dates")
    .arg(to.isValid()?to.toString(Qt::ISODate):"All dates")
    .arg(labs.value("Total")).arg(labs.value("Available")).arg(labs.value("Unavailable")).arg(labs.value("Under Maintenance"))
    .arg(bookings.total).arg(bookings.pending).arg(bookings.approved).arg(bookings.rejected).arg(bookings.cancelled).arg(bookings.completed)
    .arg(gear.value("Available")).arg(gear.value("Unavailable")).arg(gear.value("Maintenance"));
}
ServiceResult ReportService::exportPdf(const QString&filePath,const QDate&from,const QDate&to)const{
  if(filePath.trimmed().isEmpty())return ServiceResult::failure("Choose a PDF file name.");
  QPrinter printer(QPrinter::HighResolution);
  printer.setOutputFormat(QPrinter::PdfFormat);printer.setOutputFileName(filePath);
  QTextDocument document;document.setHtml(summaryHtml(from,to));document.print(&printer);
  const QFileInfo output(filePath);
  return output.exists()&&output.size()>0?ServiceResult::success():ServiceResult::failure("The report could not be written.");
}
