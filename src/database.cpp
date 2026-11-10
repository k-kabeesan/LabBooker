#include "database.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QFile>
#include <QDateTime>
#include <QHash>
#include <QStringList>

namespace {
QString nonNull(const QString& value){return value.isNull()?QStringLiteral(""):value;}
QString nowUtc(){return QDateTime::currentDateTimeUtc().toString(Qt::ISODateWithMs);}
QString sqlError(const QSqlQuery& query){return query.lastError().text();}
int roleId(UserRole role){return role==UserRole::Administrator?3:role==UserRole::Lecturer?2:1;}
UserRole roleFromInt(int id){return id==3?UserRole::Administrator:id==2?UserRole::Lecturer:UserRole::Student;}
LaboratoryStatus labStatusFrom(const QString& value){
  if(value=="Unavailable")return LaboratoryStatus::Unavailable;
  if(value=="Under Maintenance")return LaboratoryStatus::UnderMaintenance;
  return LaboratoryStatus::Available;
}
EquipmentStatus equipmentStatusFrom(const QString& value){
  if(value=="Unavailable")return EquipmentStatus::Unavailable;
  if(value=="Maintenance")return EquipmentStatus::Maintenance;
  return EquipmentStatus::Available;
}
UserRecord readUser(const QSqlQuery& q){
  UserRecord user;
  user.id=q.value(0).toLongLong(); user.eid=q.value(1).toString(); user.name=q.value(2).toString();
  user.email=q.value(3).toString(); user.faculty=q.value(4).toString(); user.department=q.value(5).toString();
  user.contact=q.value(6).toString(); user.role=roleFromInt(q.value(7).toInt());
  user.active=q.value(8).toBool(); user.mustChange=q.value(9).toBool(); user.approved=q.value(10).toBool();
  return user;
}
Booking readBooking(const QSqlQuery& q,int offset=0){
  Booking booking;
  booking.id=q.value(offset).toLongLong(); booking.reference=q.value(offset+1).toString();
  booking.userId=q.value(offset+2).toLongLong(); booking.labId=q.value(offset+3).toLongLong();
  booking.date=QDate::fromString(q.value(offset+4).toString(),Qt::ISODate);
  booking.start=QTime::fromString(q.value(offset+5).toString(),"HH:mm");
  booking.end=QTime::fromString(q.value(offset+6).toString(),"HH:mm");
  booking.purpose=q.value(offset+7).toString();
  booking.status=bookingStatusFromString(q.value(offset+8).toString());
  booking.decisionNote=q.value(offset+9).toString();
  return booking;
}
const QString bookingColumns="booking_id,reference,user_id,lab_id,booking_date,start_time,end_time,purpose,status,decision_note";
}

DatabaseManager::DatabaseManager(QString path):m_path(std::move(path)){}
DatabaseManager::~DatabaseManager(){
  const QString connection=m_db.connectionName();
  if(m_db.isOpen())m_db.close();
  m_db=QSqlDatabase();
  if(!connection.isEmpty())QSqlDatabase::removeDatabase(connection);
}
ServiceResult DatabaseManager::open(){
  if(m_path.isEmpty())return ServiceResult::failure("Database path is empty");
  if(m_db.isValid())return m_db.isOpen()?ServiceResult::success():ServiceResult::failure("Database connection was already created");
  const QString connection="labbooker_"+QString::number(reinterpret_cast<quintptr>(this));
  m_db=QSqlDatabase::addDatabase("QSQLITE",connection);
  m_db.setDatabaseName(m_path);
  if(!m_db.open())return ServiceResult::failure(m_db.lastError().text());
  auto result=execute("PRAGMA foreign_keys=ON");
  if(!result.ok)return result;
  result=execute("PRAGMA busy_timeout=5000");
  if(!result.ok)return result;
  return migrate();
}
ServiceResult DatabaseManager::execute(const QString& sql) const{
  QSqlQuery query(m_db);
  return query.exec(sql)?ServiceResult::success():ServiceResult::failure(sqlError(query));
}
ServiceResult DatabaseManager::addNotification(qint64 userId, qint64 bookingId, const QString& action, const QString& description) {
  QSqlQuery query(m_db);
  query.prepare("INSERT INTO audit_log(user_id,action,entity_type,entity_id,timestamp,description) VALUES(?,?,'Notification',?,?,?)");
  query.addBindValue(userId);
  query.addBindValue(action);
  query.addBindValue(bookingId > 0 ? QVariant(bookingId) : QVariant());
  query.addBindValue(nowUtc());
  query.addBindValue(description);
  return query.exec() ? ServiceResult::success() : ServiceResult::failure(sqlError(query));
}
ServiceResult DatabaseManager::migrate(){
  QSqlQuery versionQuery(m_db);
  if(!versionQuery.exec("PRAGMA user_version")||!versionQuery.next())return ServiceResult::failure(sqlError(versionQuery));
  int version=versionQuery.value(0).toInt();
  versionQuery.finish();
  if(version>5)return ServiceResult::failure("This database was created by a newer LabBook version.");
  if(version==0){
    QFile schema(":/database/schema.sql");
    if(!schema.open(QIODevice::ReadOnly))return ServiceResult::failure("Database schema resource is missing.");
    const QStringList statements=QString::fromUtf8(schema.readAll()).split(';',Qt::SkipEmptyParts);
    auto result=execute("BEGIN IMMEDIATE");
    if(!result.ok)return result;
    for(QString statement:statements){
      statement=statement.trimmed();
      if(statement.isEmpty())continue;
      result=execute(statement);
      if(!result.ok){execute("ROLLBACK");return result;}
    }
    result=execute("PRAGMA user_version=5");
    if(!result.ok){execute("ROLLBACK");return result;}
    result=execute("COMMIT");
    if(!result.ok){execute("ROLLBACK");return result;}
    return {};
  }
  if(version==1){
    auto result=execute("PRAGMA foreign_keys=OFF");
    if(!result.ok)return result;
    result=execute("BEGIN IMMEDIATE");
    if(!result.ok){execute("PRAGMA foreign_keys=ON");return result;}
    const QStringList migration={
      "ALTER TABLE users ADD COLUMN email TEXT NOT NULL DEFAULT ''",
      "ALTER TABLE equipment ADD COLUMN notes TEXT NOT NULL DEFAULT ''",
      "CREATE TABLE bookings_v2 (booking_id INTEGER PRIMARY KEY AUTOINCREMENT, reference TEXT NOT NULL UNIQUE, user_id INTEGER NOT NULL REFERENCES users(user_id), lab_id INTEGER NOT NULL REFERENCES laboratories(lab_id), booking_date TEXT NOT NULL, start_time TEXT NOT NULL, end_time TEXT NOT NULL, purpose TEXT NOT NULL, status TEXT NOT NULL DEFAULT 'Pending' CHECK(status IN ('Pending','Approved','Rejected','Cancelled','Completed')), reviewed_by INTEGER REFERENCES users(user_id), decision_note TEXT NOT NULL DEFAULT '', created_at TEXT NOT NULL, updated_at TEXT NOT NULL, CHECK(start_time < end_time))",
      "INSERT INTO bookings_v2(booking_id,reference,user_id,lab_id,booking_date,start_time,end_time,purpose,status,reviewed_by,decision_note,created_at,updated_at) SELECT booking_id,reference,user_id,lab_id,booking_date,start_time,end_time,purpose,status,reviewed_by,decision_note,created_at,updated_at FROM bookings",
      "DROP TABLE bookings",
      "ALTER TABLE bookings_v2 RENAME TO bookings",
      "CREATE TABLE booking_approvals (approval_id INTEGER PRIMARY KEY AUTOINCREMENT, booking_id INTEGER NOT NULL REFERENCES bookings(booking_id) ON DELETE CASCADE, admin_id INTEGER NOT NULL REFERENCES users(user_id), action TEXT NOT NULL CHECK(action IN ('Approved','Rejected','Cancelled','Completed')), comment TEXT NOT NULL DEFAULT '', action_time TEXT NOT NULL)",
      "CREATE TABLE audit_log (log_id INTEGER PRIMARY KEY AUTOINCREMENT, user_id INTEGER REFERENCES users(user_id), action TEXT NOT NULL, entity_type TEXT NOT NULL DEFAULT '', entity_id INTEGER, timestamp TEXT NOT NULL, description TEXT NOT NULL DEFAULT '')",
      "CREATE INDEX IF NOT EXISTS idx_users_name ON users(name)",
      "CREATE INDEX IF NOT EXISTS idx_bookings_lab_time ON bookings(lab_id,booking_date,start_time,end_time,status)",
      "CREATE INDEX IF NOT EXISTS idx_bookings_user ON bookings(user_id,status,booking_date)",
      "CREATE INDEX IF NOT EXISTS idx_booking_approvals_booking ON booking_approvals(booking_id,action_time)",
      "PRAGMA user_version=2"
    };
    for(const auto& statement:migration){
      result=execute(statement);
      if(!result.ok){execute("ROLLBACK");execute("PRAGMA foreign_keys=ON");return result;}
    }
    result=execute("COMMIT");
    if(!result.ok){execute("ROLLBACK");execute("PRAGMA foreign_keys=ON");return result;}
    result=execute("PRAGMA foreign_keys=ON");
    if(!result.ok)return result;
    QSqlQuery check(m_db);
    if(!check.exec("PRAGMA foreign_key_check"))return ServiceResult::failure(sqlError(check));
    if(check.next())return ServiceResult::failure("Database migration found a broken foreign-key reference.");
    check.finish();
    return migrate();
  }
  if(version==2){
    auto result=transaction([&]{
      auto changed=execute("ALTER TABLE users ADD COLUMN approved INTEGER NOT NULL DEFAULT 1 CHECK(approved IN (0,1))");
      if(!changed.ok)return changed;
      return execute("PRAGMA user_version=3");
    });
    return result.ok ? migrate() : result;
  }
  if(version==3){
    auto result=transaction([&]{
      for (const auto& sql : {
        "CREATE TABLE IF NOT EXISTS booking_schedule (id INTEGER PRIMARY KEY CHECK(id = 1), morning_start TEXT NOT NULL DEFAULT '08:30', afternoon_start TEXT NOT NULL DEFAULT '13:30')",
        "INSERT OR IGNORE INTO booking_schedule(id,morning_start,afternoon_start) VALUES(1,'08:30','13:30')",
        "CREATE TABLE IF NOT EXISTS closed_dates (closed_date TEXT PRIMARY KEY)",
        "PRAGMA user_version=4"
      }) {
        const auto result = execute(sql);
        if (!result.ok) return result;
      }
      return ServiceResult::success();
    });
    return result.ok ? migrate() : result;
  }
  if(version==4){
    return transaction([&]{
      for(const auto& sql : {
        "ALTER TABLE users ADD COLUMN recovery_question TEXT NOT NULL DEFAULT ''",
        "ALTER TABLE users ADD COLUMN recovery_hash TEXT NOT NULL DEFAULT ''",
        "ALTER TABLE users ADD COLUMN recovery_salt TEXT NOT NULL DEFAULT ''",
        "PRAGMA user_version=5"
      }){const auto result=execute(sql);if(!result.ok)return result;}
      return ServiceResult::success();
    });
  }
  return {};
}
ServiceResult DatabaseManager::transaction(const std::function<ServiceResult()>& fn){
  auto result=execute("BEGIN IMMEDIATE");
  if(!result.ok)return result;
  result=fn();
  if(!result.ok){execute("ROLLBACK");return result;}
  result=execute("COMMIT");
  if(!result.ok){execute("ROLLBACK");return result;}
  return result;
}
bool DatabaseManager::hasUsers() const{
  QSqlQuery q(m_db);
  return q.exec("SELECT 1 FROM users LIMIT 1")&&q.next();
}
QVector<FixedBookingSlot> DatabaseManager::bookingSlots() const {
  QTime morning(8, 30), afternoon(13, 30);
  QSqlQuery query(m_db);
  if (query.exec("SELECT morning_start, afternoon_start FROM booking_schedule WHERE id=1") && query.next()) {
    const QTime a = QTime::fromString(query.value(0).toString(), "HH:mm");
    const QTime b = QTime::fromString(query.value(1).toString(), "HH:mm");
    if (a.isValid() && b.isValid()) { morning = a; afternoon = b; }
  }
  const QVector<QTime> starts{morning, morning.addSecs(7200), afternoon, afternoon.addSecs(7200)};
  QVector<FixedBookingSlot> result;
  for (int i = 0; i < starts.size(); ++i) {
    const QTime end = starts[i].addSecs(7200);
    const QString label = starts[i].toString("h:mm AP") + " – " + end.toString("h:mm AP");
    result.push_back({i + 1, starts[i], end, label, QString("Slot %1 (%2)").arg(i + 1).arg(label)});
  }
  return result;
}
bool DatabaseManager::isClosedDate(const QDate& date) const {
  if (isWeekend(date)) return true;
  QSqlQuery query(m_db);
  query.prepare("SELECT 1 FROM closed_dates WHERE closed_date=?");
  query.addBindValue(date.toString(Qt::ISODate));
  return query.exec() && query.next();
}
QVector<QDate> DatabaseManager::closedDates() const {
  QVector<QDate> dates;
  QSqlQuery query(m_db);
  if (query.exec("SELECT closed_date FROM closed_dates ORDER BY closed_date"))
    while (query.next()) dates.push_back(QDate::fromString(query.value(0).toString(), Qt::ISODate));
  return dates;
}
ServiceResult DatabaseManager::setBookingSchedule(const QTime& morningStart, const QTime& afternoonStart) {
  if (!morningStart.isValid() || !afternoonStart.isValid() || morningStart < QTime(6, 0) ||
      morningStart.secsTo(afternoonStart) < 14400 || afternoonStart > QTime(18, 0))
    return ServiceResult::failure("Choose two morning sessions and two afternoon sessions, with a break between them.");
  QSqlQuery active(m_db);
  active.prepare("SELECT 1 FROM bookings WHERE status IN ('Pending','Approved') AND booking_date >= ? LIMIT 1");
  active.addBindValue(QDate::currentDate().toString(Qt::ISODate));
  if (!active.exec()) return ServiceResult::failure(sqlError(active));
  if (active.next()) return ServiceResult::failure("Resolve upcoming pending and approved bookings before changing session times.");
  QSqlQuery query(m_db);
  query.prepare("UPDATE booking_schedule SET morning_start=?, afternoon_start=? WHERE id=1");
  query.addBindValue(morningStart.toString("HH:mm"));
  query.addBindValue(afternoonStart.toString("HH:mm"));
  return query.exec() ? ServiceResult::success() : ServiceResult::failure(sqlError(query));
}
ServiceResult DatabaseManager::setClosedDate(const QDate& date, bool closed) {
  if (!date.isValid()) return ServiceResult::failure("Choose a valid closure date.");
  if (closed) {
    QSqlQuery active(m_db);
    active.prepare("SELECT 1 FROM bookings WHERE booking_date=? AND status IN ('Pending','Approved') LIMIT 1");
    active.addBindValue(date.toString(Qt::ISODate));
    if (!active.exec()) return ServiceResult::failure(sqlError(active));
    if (active.next()) return ServiceResult::failure("Resolve pending and approved bookings on this date before closing it.");
  }
  QSqlQuery query(m_db);
  query.prepare(closed ? "INSERT OR IGNORE INTO closed_dates(closed_date) VALUES(?)" : "DELETE FROM closed_dates WHERE closed_date=?");
  query.addBindValue(date.toString(Qt::ISODate));
  return query.exec() ? ServiceResult::success() : ServiceResult::failure(sqlError(query));
}
QVector<UserRecord> DatabaseManager::users(QString filter,bool includeInactive) const{
  QVector<UserRecord> out;
  QSqlQuery q(m_db);
  q.prepare(QString("SELECT user_id,eid,name,email,faculty,department,contact,role_id,active,must_change_password,approved FROM users WHERE (eid LIKE :filter OR name LIKE :filter OR email LIKE :filter) %1 ORDER BY name")
    .arg(includeInactive?"":"AND active=1 AND approved=1"));
  q.bindValue(":filter","%"+filter.trimmed()+"%");
  if(!q.exec())return out;
  while(q.next())out.push_back(readUser(q));
  return out;
}
std::optional<UserRecord> DatabaseManager::userByEid(const QString& eid) const{
  QSqlQuery q(m_db);
  q.prepare("SELECT user_id,eid,name,email,faculty,department,contact,role_id,active,must_change_password,approved FROM users WHERE eid=?");
  q.addBindValue(eid.trimmed());
  if(!q.exec()||!q.next())return std::nullopt;
  return readUser(q);
}
std::optional<UserRecord> DatabaseManager::userById(qint64 id) const{
  QSqlQuery q(m_db);
  q.prepare("SELECT user_id,eid,name,email,faculty,department,contact,role_id,active,must_change_password,approved FROM users WHERE user_id=?");
  q.addBindValue(id);
  if(!q.exec()||!q.next())return std::nullopt;
  return readUser(q);
}
ServiceResult DatabaseManager::insertUser(const UserRecord& user,const QString& hash,const QString& salt,int iterations,bool mustChange,bool approved){
  QSqlQuery q(m_db);
  q.prepare("INSERT INTO users(eid,name,email,faculty,department,contact,role_id,password_hash,password_salt,password_iterations,must_change_password,approved,created_at,updated_at) VALUES(?,?,?,?,?,?,?,?,?,?,?,?,?,?)");
  q.addBindValue(user.eid.trimmed()); q.addBindValue(user.name.trimmed()); q.addBindValue(nonNull(user.email.trimmed()));
  q.addBindValue(nonNull(user.faculty.trimmed())); q.addBindValue(nonNull(user.department.trimmed())); q.addBindValue(nonNull(user.contact.trimmed()));
  q.addBindValue(roleId(user.role)); q.addBindValue(hash); q.addBindValue(salt); q.addBindValue(iterations);
  q.addBindValue(mustChange?1:0);q.addBindValue(approved?1:0);q.addBindValue(nowUtc());q.addBindValue(nowUtc());
  if(!q.exec())return ServiceResult::failure(sqlError(q));
  const qint64 id=q.lastInsertId().toLongLong();
  if(user.role==UserRole::Student){QSqlQuery profile(m_db);profile.prepare("INSERT INTO student_profiles(user_id) VALUES(?)");profile.addBindValue(id);if(!profile.exec())return ServiceResult::failure(sqlError(profile));}
  if(user.role==UserRole::Lecturer){QSqlQuery profile(m_db);profile.prepare("INSERT INTO lecturer_profiles(user_id) VALUES(?)");profile.addBindValue(id);if(!profile.exec())return ServiceResult::failure(sqlError(profile));}
  return {};
}
ServiceResult DatabaseManager::updateUser(const UserRecord& user){
  QSqlQuery q(m_db);
  q.prepare("UPDATE users SET eid=?,name=?,email=?,faculty=?,department=?,contact=?,role_id=?,updated_at=? WHERE user_id=?");
  q.addBindValue(user.eid.trimmed());q.addBindValue(user.name.trimmed());q.addBindValue(nonNull(user.email.trimmed()));
  q.addBindValue(nonNull(user.faculty.trimmed()));q.addBindValue(nonNull(user.department.trimmed()));q.addBindValue(nonNull(user.contact.trimmed()));
  q.addBindValue(roleId(user.role));q.addBindValue(nowUtc());q.addBindValue(user.id);
  if(!q.exec())return ServiceResult::failure(sqlError(q));
  if(q.numRowsAffected()==0)return ServiceResult::failure("User was not found.");
  return transaction([&]{
    if(user.role==UserRole::Student){QSqlQuery p(m_db);p.prepare("INSERT OR IGNORE INTO student_profiles(user_id) VALUES(?)");p.addBindValue(user.id);if(!p.exec())return ServiceResult::failure(sqlError(p));}
    if(user.role==UserRole::Lecturer){QSqlQuery p(m_db);p.prepare("INSERT OR IGNORE INTO lecturer_profiles(user_id) VALUES(?)");p.addBindValue(user.id);if(!p.exec())return ServiceResult::failure(sqlError(p));}
    return ServiceResult::success();
  });
}
ServiceResult DatabaseManager::setPassword(qint64 id,const QString& hash,const QString& salt,int iterations,bool mustChange){
  QSqlQuery q(m_db);q.prepare("UPDATE users SET password_hash=?,password_salt=?,password_iterations=?,must_change_password=?,updated_at=? WHERE user_id=?");
  q.addBindValue(hash);q.addBindValue(salt);q.addBindValue(iterations);q.addBindValue(mustChange?1:0);q.addBindValue(nowUtc());q.addBindValue(id);
  return q.exec()?ServiceResult::success():ServiceResult::failure(sqlError(q));
}
ServiceResult DatabaseManager::setUserActive(qint64 id,bool active){
  QSqlQuery q(m_db);q.prepare("UPDATE users SET active=?,updated_at=? WHERE user_id=?");q.addBindValue(active?1:0);q.addBindValue(nowUtc());q.addBindValue(id);
  if(!q.exec())return ServiceResult::failure(sqlError(q));
  return q.numRowsAffected()?ServiceResult::success():ServiceResult::failure("User was not found.");
}
ServiceResult DatabaseManager::setUserApproved(qint64 id,bool approved){
  QSqlQuery q(m_db);q.prepare("UPDATE users SET approved=?,updated_at=? WHERE user_id=?");q.addBindValue(approved?1:0);q.addBindValue(nowUtc());q.addBindValue(id);
  if(!q.exec())return ServiceResult::failure(sqlError(q));
  if (!q.numRowsAffected()) return ServiceResult::failure("User was not found.");
  return approved ? addNotification(id, 0, "Account approved", "Your account is approved. You can now sign in.") : ServiceResult::success();
}
QVector<Laboratory> DatabaseManager::laboratories(bool includeInactive) const{
  QVector<Laboratory> out;
  QSqlQuery q(m_db);
  q.prepare(QString("SELECT lab_id,lab_name,lab_number,location,department,capacity,description,status,active FROM laboratories %1 ORDER BY lab_name")
    .arg(includeInactive?"":"WHERE active=1"));
  if(!q.exec())return out;
  while(q.next()){
    Laboratory lab;lab.id=q.value(0).toLongLong();lab.name=q.value(1).toString();lab.number=q.value(2).toString();
    lab.location=q.value(3).toString();lab.department=q.value(4).toString();lab.capacity=q.value(5).toInt();
    lab.description=q.value(6).toString();lab.status=labStatusFrom(q.value(7).toString());lab.active=q.value(8).toBool();
    out.push_back(lab);
  }
  return out;
}
ServiceResult DatabaseManager::addLaboratory(const Laboratory& lab){
  QSqlQuery q(m_db);q.prepare("INSERT INTO laboratories(lab_name,lab_number,location,department,capacity,description,status,active,created_at,updated_at) VALUES(?,?,?,?,?,?,?,?,?,?)");
  q.addBindValue(lab.name.trimmed());q.addBindValue(lab.number.trimmed());q.addBindValue(nonNull(lab.location.trimmed()));q.addBindValue(nonNull(lab.department.trimmed()));
  q.addBindValue(lab.capacity);q.addBindValue(nonNull(lab.description.trimmed()));q.addBindValue(toString(lab.status));q.addBindValue(lab.active?1:0);q.addBindValue(nowUtc());q.addBindValue(nowUtc());
  return q.exec()?ServiceResult::success():ServiceResult::failure(sqlError(q));
}
ServiceResult DatabaseManager::updateLaboratory(const Laboratory& lab){
  QSqlQuery q(m_db);q.prepare("UPDATE laboratories SET lab_name=?,lab_number=?,location=?,department=?,capacity=?,description=?,status=?,active=?,updated_at=? WHERE lab_id=?");
  q.addBindValue(lab.name.trimmed());q.addBindValue(lab.number.trimmed());q.addBindValue(nonNull(lab.location.trimmed()));q.addBindValue(nonNull(lab.department.trimmed()));
  q.addBindValue(lab.capacity);q.addBindValue(nonNull(lab.description.trimmed()));q.addBindValue(toString(lab.status));q.addBindValue(lab.active?1:0);q.addBindValue(nowUtc());q.addBindValue(lab.id);
  if(!q.exec())return ServiceResult::failure(sqlError(q));
  return q.numRowsAffected()?ServiceResult::success():ServiceResult::failure("Laboratory was not found.");
}
ServiceResult DatabaseManager::setLaboratoryActive(qint64 id,bool active){
  QSqlQuery q(m_db);q.prepare("UPDATE laboratories SET active=?,updated_at=? WHERE lab_id=?");q.addBindValue(active?1:0);q.addBindValue(nowUtc());q.addBindValue(id);
  if(!q.exec())return ServiceResult::failure(sqlError(q));
  return q.numRowsAffected()?ServiceResult::success():ServiceResult::failure("Laboratory was not found.");
}
QVector<Equipment> DatabaseManager::equipment(bool includeInactive) const{
  QVector<Equipment> out;QSqlQuery q(m_db);
  q.prepare(QString("SELECT equipment_id,lab_id,equipment_name,total_qty,available_qty,condition,status,notes,active FROM equipment %1 ORDER BY equipment_name")
    .arg(includeInactive?"":"WHERE active=1"));
  if(!q.exec())return out;
  while(q.next()){
    Equipment e;e.id=q.value(0).toLongLong();e.labId=q.value(1).toLongLong();e.name=q.value(2).toString();
    e.totalQty=q.value(3).toInt();e.availableQty=q.value(4).toInt();e.condition=q.value(5).toString();
    e.status=equipmentStatusFrom(q.value(6).toString());e.notes=q.value(7).toString();e.active=q.value(8).toBool();out.push_back(e);
  }
  return out;
}
ServiceResult DatabaseManager::addEquipment(const Equipment& e){
  QSqlQuery q(m_db);q.prepare("INSERT INTO equipment(lab_id,equipment_name,total_qty,available_qty,condition,status,notes,active,created_at,updated_at) VALUES(?,?,?,?,?,?,?,?,?,?)");
  q.addBindValue(e.labId);q.addBindValue(e.name.trimmed());q.addBindValue(e.totalQty);q.addBindValue(e.availableQty);
  q.addBindValue(nonNull(e.condition.trimmed()));q.addBindValue(toString(e.status));q.addBindValue(nonNull(e.notes.trimmed()));q.addBindValue(e.active?1:0);q.addBindValue(nowUtc());q.addBindValue(nowUtc());
  return q.exec()?ServiceResult::success():ServiceResult::failure(sqlError(q));
}
ServiceResult DatabaseManager::updateEquipment(const Equipment& e){
  QSqlQuery q(m_db);q.prepare("UPDATE equipment SET lab_id=?,equipment_name=?,total_qty=?,available_qty=?,condition=?,status=?,notes=?,active=?,updated_at=? WHERE equipment_id=?");
  q.addBindValue(e.labId);q.addBindValue(e.name.trimmed());q.addBindValue(e.totalQty);q.addBindValue(e.availableQty);
  q.addBindValue(nonNull(e.condition.trimmed()));q.addBindValue(toString(e.status));q.addBindValue(nonNull(e.notes.trimmed()));q.addBindValue(e.active?1:0);q.addBindValue(nowUtc());q.addBindValue(e.id);
  if(!q.exec())return ServiceResult::failure(sqlError(q));
  return q.numRowsAffected()?ServiceResult::success():ServiceResult::failure("Equipment record was not found.");
}
ServiceResult DatabaseManager::setEquipmentActive(qint64 id,bool active){
  QSqlQuery q(m_db);q.prepare("UPDATE equipment SET active=?,updated_at=? WHERE equipment_id=?");q.addBindValue(active?1:0);q.addBindValue(nowUtc());q.addBindValue(id);
  if(!q.exec())return ServiceResult::failure(sqlError(q));
  return q.numRowsAffected()?ServiceResult::success():ServiceResult::failure("Equipment record was not found.");
}
QVector<Booking> DatabaseManager::bookingsForUser(qint64 id) const{
  QVector<Booking> out;QSqlQuery q(m_db);q.prepare("SELECT "+bookingColumns+" FROM bookings WHERE user_id=? ORDER BY booking_date,start_time");
  q.addBindValue(id);if(!q.exec())return out;while(q.next())out.push_back(readBooking(q));return out;
}
QVector<Booking> DatabaseManager::allBookings(bool pendingOnly) const{
  QVector<Booking> out;QSqlQuery q(m_db);
  q.prepare("SELECT "+bookingColumns+" FROM bookings "+(pendingOnly?"WHERE status='Pending' ":"")+"ORDER BY booking_date,start_time");
  if(!q.exec())return out;while(q.next())out.push_back(readBooking(q));return out;
}
std::optional<Booking> DatabaseManager::bookingById(qint64 id) const{
  QSqlQuery q(m_db);q.prepare("SELECT "+bookingColumns+" FROM bookings WHERE booking_id=?");q.addBindValue(id);
  if(!q.exec()||!q.next())return std::nullopt;return readBooking(q);
}
QVector<BookingApproval> DatabaseManager::bookingApprovals(qint64 bookingId) const{
  QVector<BookingApproval> out;QSqlQuery q(m_db);
  q.prepare("SELECT a.approval_id,a.booking_id,a.admin_id,a.action,a.comment,a.action_time,u.name FROM booking_approvals a JOIN users u ON u.user_id=a.admin_id WHERE a.booking_id=? ORDER BY a.action_time");
  q.addBindValue(bookingId);if(!q.exec())return out;
  while(q.next()){BookingApproval a;a.id=q.value(0).toLongLong();a.bookingId=q.value(1).toLongLong();a.adminId=q.value(2).toLongLong();a.action=q.value(3).toString();a.comment=q.value(4).toString();a.actionTime=q.value(5).toString();a.adminName=q.value(6).toString();out.push_back(a);}
  return out;
}
QVector<AppNotification> DatabaseManager::notificationsForUser(qint64 userId) const {
  QVector<AppNotification> notifications;
  QSqlQuery query(m_db);
  query.prepare("SELECT log_id,entity_id,action,description,timestamp FROM audit_log WHERE user_id=? AND entity_type='Notification' ORDER BY timestamp DESC,log_id DESC");
  query.addBindValue(userId);
  if (!query.exec()) return notifications;
  while (query.next()) notifications.push_back({query.value(0).toLongLong(), query.value(1).toLongLong(),
    query.value(2).toString(), query.value(3).toString(), query.value(4).toString()});
  return notifications;
}
QVector<BookedEquipment> DatabaseManager::equipmentForBooking(qint64 bookingId) const{
  QVector<BookedEquipment> out;QSqlQuery q(m_db);
  q.prepare("SELECT be.equipment_id,e.equipment_name,be.qty_requested FROM booking_equipment be JOIN equipment e ON e.equipment_id=be.equipment_id WHERE be.booking_id=? ORDER BY e.equipment_name");
  q.addBindValue(bookingId);if(!q.exec())return out;
  while(q.next()){BookedEquipment item;item.equipmentId=q.value(0).toLongLong();item.equipmentName=q.value(1).toString();item.quantity=q.value(2).toInt();out.push_back(item);}
  return out;
}
bool DatabaseManager::isAvailable(qint64 labId,const QDate& date,const QTime& start,const QTime& end,qint64 exclude) const{
  if(labId<=0||!date.isValid()||!start.isValid()||!end.isValid()||start>=end)return false;
  QSqlQuery q(m_db);
  q.prepare("SELECT COUNT(*) FROM laboratories l WHERE l.lab_id=? AND l.active=1 AND l.status='Available' AND NOT EXISTS (SELECT 1 FROM bookings b WHERE b.lab_id=l.lab_id AND b.booking_date=? AND b.status IN ('Pending','Approved') AND b.start_time < ? AND b.end_time > ? AND b.booking_id<>?)");
  q.addBindValue(labId);q.addBindValue(date.toString(Qt::ISODate));q.addBindValue(end.toString("HH:mm"));q.addBindValue(start.toString("HH:mm"));q.addBindValue(exclude);
  return q.exec()&&q.next()&&q.value(0).toInt()==1;
}
ServiceResult DatabaseManager::checkEquipmentAvailability(qint64 labId,const QDate& date,const QTime& start,const QTime& end,const QVector<QPair<qint64,int>>& requested,qint64 exclude) const{
  if(requested.isEmpty())return {};
  QHash<qint64,int> totals;
  for(const auto& item:requested){
    if(item.first<=0||item.second<=0)return ServiceResult::failure("Equipment quantities must be positive.");
    if(totals.contains(item.first))return ServiceResult::failure("Choose each equipment item only once.");
    totals.insert(item.first,item.second);
    QSqlQuery available(m_db);available.prepare("SELECT available_qty FROM equipment WHERE equipment_id=? AND lab_id=? AND active=1 AND status='Available'");
    available.addBindValue(item.first);available.addBindValue(labId);
    if(!available.exec()||!available.next())return ServiceResult::failure("A selected equipment item is unavailable for this laboratory.");
    const int physical=available.value(0).toInt();
    QSqlQuery reserved(m_db);reserved.prepare("SELECT COALESCE(SUM(be.qty_requested),0) FROM booking_equipment be JOIN bookings b ON b.booking_id=be.booking_id WHERE be.equipment_id=? AND b.booking_date=? AND b.status IN ('Pending','Approved') AND b.start_time < ? AND b.end_time > ? AND b.booking_id<>?");
    reserved.addBindValue(item.first);reserved.addBindValue(date.toString(Qt::ISODate));reserved.addBindValue(end.toString("HH:mm"));reserved.addBindValue(start.toString("HH:mm"));reserved.addBindValue(exclude);
    if(!reserved.exec()||!reserved.next())return ServiceResult::failure(sqlError(reserved));
    if(item.second>physical-reserved.value(0).toInt())return ServiceResult::failure("Not enough equipment is available for the selected time.");
  }
  return {};
}
ServiceResult DatabaseManager::insertBooking(const BookingRequest& request,const QString& reference,qint64* idOut){
  QSqlQuery q(m_db);q.prepare("INSERT INTO bookings(reference,user_id,lab_id,booking_date,start_time,end_time,purpose,created_at,updated_at) VALUES(?,?,?,?,?,?,?,?,?)");
  q.addBindValue(reference);q.addBindValue(request.userId);q.addBindValue(request.labId);q.addBindValue(request.date.toString(Qt::ISODate));
  q.addBindValue(request.start.toString("HH:mm"));q.addBindValue(request.end.toString("HH:mm"));q.addBindValue(request.purpose.trimmed());q.addBindValue(nowUtc());q.addBindValue(nowUtc());
  if(!q.exec())return ServiceResult::failure(sqlError(q));
  const qint64 id=q.lastInsertId().toLongLong();if(idOut)*idOut=id;
  for(const auto& item:request.equipment){
    QSqlQuery line(m_db);line.prepare("INSERT INTO booking_equipment(booking_id,equipment_id,qty_requested) VALUES(?,?,?)");
    line.addBindValue(id);line.addBindValue(item.first);line.addBindValue(item.second);
    if(!line.exec())return ServiceResult::failure(sqlError(line));
  }
  return {};
}
ServiceResult DatabaseManager::updateBooking(const Booking& booking,const BookingRequest& request){
  QSqlQuery q(m_db);q.prepare("UPDATE bookings SET lab_id=?,booking_date=?,start_time=?,end_time=?,purpose=?,status='Pending',reviewed_by=NULL,decision_note='',updated_at=? WHERE booking_id=? AND user_id=? AND status='Pending'");
  q.addBindValue(request.labId);q.addBindValue(request.date.toString(Qt::ISODate));q.addBindValue(request.start.toString("HH:mm"));q.addBindValue(request.end.toString("HH:mm"));
  q.addBindValue(request.purpose.trimmed());q.addBindValue(nowUtc());q.addBindValue(booking.id);q.addBindValue(request.userId);
  if(!q.exec())return ServiceResult::failure(sqlError(q));
  if(q.numRowsAffected()==0)return ServiceResult::failure("Only your pending bookings can be changed.");
  QSqlQuery remove(m_db);remove.prepare("DELETE FROM booking_equipment WHERE booking_id=?");remove.addBindValue(booking.id);
  if(!remove.exec())return ServiceResult::failure(sqlError(remove));
  for(const auto& item:request.equipment){
    QSqlQuery line(m_db);line.prepare("INSERT INTO booking_equipment(booking_id,equipment_id,qty_requested) VALUES(?,?,?)");
    line.addBindValue(booking.id);line.addBindValue(item.first);line.addBindValue(item.second);
    if(!line.exec())return ServiceResult::failure(sqlError(line));
  }
  return addNotification(booking.userId, booking.id, "Booking changed", "Booking " + booking.reference + " was updated and remains pending review.");
}
ServiceResult DatabaseManager::setBookingStatus(qint64 id,BookingStatus status,qint64 reviewer,const QString& note){
  const auto existing = bookingById(id);
  if (!existing) return ServiceResult::failure("Booking was not found.");
  const QString action=toString(status);
  if(status!=BookingStatus::Approved&&status!=BookingStatus::Rejected&&status!=BookingStatus::Cancelled&&status!=BookingStatus::Completed)
    return ServiceResult::failure("That booking action is not allowed.");
  QSqlQuery q(m_db);
  const QString condition=(status==BookingStatus::Cancelled)?"status IN ('Pending','Approved')":
    status==BookingStatus::Completed?"status='Approved'":"status='Pending'";
  q.prepare("UPDATE bookings SET status=?,reviewed_by=?,decision_note=?,updated_at=? WHERE booking_id=? AND "+condition);
  q.addBindValue(action);q.addBindValue(reviewer);q.addBindValue(nonNull(note.trimmed()));q.addBindValue(nowUtc());q.addBindValue(id);
  if(!q.exec())return ServiceResult::failure(sqlError(q));
  if(q.numRowsAffected()==0)return ServiceResult::failure("The booking is no longer eligible for that action.");
  QSqlQuery roleQuery(m_db);roleQuery.prepare("SELECT role_id FROM users WHERE user_id=?");roleQuery.addBindValue(reviewer);
  if(!roleQuery.exec()||!roleQuery.next())return ServiceResult::failure(sqlError(roleQuery));
  if(status==BookingStatus::Cancelled&&roleQuery.value(0).toInt()!=3){
    QSqlQuery audit(m_db);audit.prepare("INSERT INTO audit_log(user_id,action,entity_type,entity_id,timestamp,description) VALUES(?,?,?,?,?,?)");
    audit.addBindValue(reviewer);audit.addBindValue("Cancelled");audit.addBindValue("Booking");audit.addBindValue(id);audit.addBindValue(nowUtc());audit.addBindValue(nonNull(note.trimmed()));
    if (!audit.exec()) return ServiceResult::failure(sqlError(audit));
    return addNotification(existing->userId, id, "Booking cancelled", "Booking " + existing->reference + " was cancelled.");
  }
  QSqlQuery history(m_db);history.prepare("INSERT INTO booking_approvals(booking_id,admin_id,action,comment,action_time) VALUES(?,?,?,?,?)");
  history.addBindValue(id);history.addBindValue(reviewer);history.addBindValue(action);history.addBindValue(nonNull(note.trimmed()));history.addBindValue(nowUtc());
  if (!history.exec()) return ServiceResult::failure(sqlError(history));
  return addNotification(existing->userId, id, "Booking " + action.toLower(),
    "Booking " + existing->reference + " was " + action.toLower() + (note.trimmed().isEmpty() ? "." : ": " + note.trimmed()));
}
ServiceResult DatabaseManager::cancelBooking(qint64 id,qint64 actorId,const QString& note){
  return setBookingStatus(id,BookingStatus::Cancelled,actorId,note);
}
int DatabaseManager::count(const QString& table,const QString& where) const{
  static const QStringList allowed={"users","laboratories","equipment","bookings"};
  if(!allowed.contains(table))return 0;
  QSqlQuery q(m_db);
  const QString sql="SELECT COUNT(*) FROM "+table+(where.trimmed().isEmpty()?"":" WHERE "+where);
  return q.exec(sql)&&q.next()?q.value(0).toInt():0;
}
