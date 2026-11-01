#include <QtTest>
#include "services.h"
#include <QTemporaryDir>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QUuid>
#include <QFile>

class CoreTests:public QObject{
  Q_OBJECT
private slots:
  void init(){
    m_path=m_dir.path()+"/"+QUuid::createUuid().toString(QUuid::WithoutBraces)+".sqlite";
    m_db=std::make_unique<DatabaseManager>(m_path);QVERIFY2(m_db->open().ok,"database open");
    UserRecord admin;admin.eid="ADMIN";admin.name="Administrator";admin.role=UserRole::Administrator;
    const auto result=AuthService(*m_db).bootstrap(admin,"correct-password");if(!result.ok)qFatal("%s",qPrintable(result.error));
    m_admin=AuthService(*m_db).authenticate("ADMIN","correct-password");QVERIFY(m_admin);
  }
  void cleanup(){m_admin.reset();m_db.reset();}
  void authenticationAndPermissions(){
    UserRecord student;student.eid="STU1";student.name="Student One";student.email="student@example.edu";student.role=UserRole::Student;
    auto created=AuthService(*m_db).createUser(student,"temporary-password");QVERIFY2(created.ok,qPrintable(created.error));
    QString error;auto valid=AuthService(*m_db).authenticate("STU1","temporary-password",&error);QVERIFY(valid);QVERIFY(valid->mustChangePassword());
    QVERIFY(hasPermission(*valid,Permission::CreateBooking));QVERIFY(!hasPermission(*valid,Permission::ManageUsers));
    QVERIFY(!AuthService(*m_db).authenticate("STU1","wrong-password",&error));QVERIFY(!error.isEmpty());
    QVERIFY(!AuthService(*m_db).authenticate("missing","anything",&error));
    QVERIFY(!AuthService(*m_db).createUser(student,"short").ok);
    QVERIFY(AuthService(*m_db).changePassword(valid->id(),"temporary-password","new-password").ok);
    QVERIFY(AuthService(*m_db).authenticate("STU1","new-password"));
  }
  void accountRegistrationRequiresAdministratorApproval(){
    UserRecord request;request.eid="NEWSTU";request.name="New Student";request.email="new.student@example.edu";request.role=UserRole::Student;
    const auto submitted=AuthService(*m_db).registerAccount(request,"student-password");QVERIFY2(submitted.ok,qPrintable(submitted.error));
    const auto pending=m_db->userByEid("NEWSTU");QVERIFY(pending);QVERIFY(!pending->approved);
    QString error;QVERIFY(!AuthService(*m_db).authenticate("NEWSTU","student-password",&error));QVERIFY(error.contains("approval",Qt::CaseInsensitive));
    auto requester=createStudent();QVERIFY(!UserManager(*m_db).approve(*requester,pending->id).ok);
    const auto approved=UserManager(*m_db).approve(*m_admin,pending->id);QVERIFY2(approved.ok,qPrintable(approved.error));
    const auto user=AuthService(*m_db).authenticate("NEWSTU","student-password",&error);QVERIFY2(user,qPrintable(error));
    QVERIFY(m_db->userByEid("NEWSTU")->approved);
    QCOMPARE(m_db->notificationsForUser(user->id()).size(), 1);
    QCOMPARE(m_db->notificationsForUser(user->id()).first().action, QString("Account approved"));
  }
  void overlapBoundariesAndIsolation(){
    auto user=createStudent();auto lab=addLab("L1");auto other=addLab("L2");
    const auto date=nextWeekday(QDate::currentDate().addDays(1));BookingService service(*m_db);
    // 4 fixed 2-hour slots
    QVERIFY(service.create(*user,request(lab.id,date,QTime(8,30),QTime(10,30))).ok);
    QVERIFY(service.create(*user,request(lab.id,date,QTime(10,30),QTime(12,30))).ok);
    QVERIFY(service.create(*user,request(lab.id,date,QTime(13,30),QTime(15,30))).ok);
    QVERIFY(service.create(*user,request(lab.id,date,QTime(15,30),QTime(17,30))).ok);

    // Double booking on same lab and same date must fail
    QVERIFY(!service.create(*user,request(lab.id,date,QTime(8,30),QTime(10,30))).ok);
    QVERIFY(!service.create(*user,request(lab.id,date,QTime(10,30),QTime(12,30))).ok);
    QVERIFY(!service.create(*user,request(lab.id,date,QTime(13,30),QTime(15,30))).ok);
    QVERIFY(!service.create(*user,request(lab.id,date,QTime(15,30),QTime(17,30))).ok);

    // Different laboratory can be booked for the same date and time slot
    QVERIFY(service.create(*user,request(other.id,date,QTime(8,30),QTime(10,30))).ok);
    QVERIFY(service.create(*user,request(other.id,date,QTime(10,30),QTime(12,30))).ok);

    // Different weekday can be booked for the same laboratory and slot
    const auto otherDate=nextWeekday(date.addDays(1));
    QVERIFY(service.create(*user,request(lab.id,otherDate,QTime(8,30),QTime(10,30))).ok);
  }
  void availabilityAndBookingHours(){
    auto user=createStudent();auto lab=addLab("HOURS");const auto date=nextWeekday(QDate::currentDate().addDays(2));
    AvailabilityService availability(*m_db);BookingService bookings(*m_db);

    // The 4 fixed 2-hour slots are valid
    QVERIFY(availability.validateRequest(lab.id,date,QTime(8,30),QTime(10,30)).ok);
    QVERIFY(availability.validateRequest(lab.id,date,QTime(10,30),QTime(12,30)).ok);
    QVERIFY(availability.validateRequest(lab.id,date,QTime(13,30),QTime(15,30)).ok);
    QVERIFY(availability.validateRequest(lab.id,date,QTime(15,30),QTime(17,30)).ok);

    // Duration must be exactly 2 hours conforming to fixed slots
    auto non2Hour=availability.validateRequest(lab.id,date,QTime(8,30),QTime(9,30));
    QVERIFY(!non2Hour.ok);
    QVERIFY(non2Hour.error.contains("2 hours",Qt::CaseInsensitive));
    QVERIFY(!availability.validateRequest(lab.id,date,QTime(8,30),QTime(11,30)).ok);
    QVERIFY(!availability.validateRequest(lab.id,date,QTime(9,0),QTime(11,0)).ok);

    // Lunch break 12:30 PM - 1:30 PM is rejected
    auto lunchOverlap1=availability.validateRequest(lab.id,date,QTime(12,30),QTime(14,30));
    QVERIFY(!lunchOverlap1.ok);
    QVERIFY(lunchOverlap1.error.contains("lunch",Qt::CaseInsensitive));
    auto lunchOverlap2=availability.validateRequest(lab.id,date,QTime(11,30),QTime(13,30));
    QVERIFY(!lunchOverlap2.ok);
    QVERIFY(lunchOverlap2.error.contains("lunch",Qt::CaseInsensitive));

    // Operating hours 8:30 AM to 5:30 PM
    QVERIFY(!availability.validateRequest(lab.id,date,QTime(8,0),QTime(10,0)).ok);
    QVERIFY(!availability.validateRequest(lab.id,date,QTime(16,0),QTime(18,0)).ok);

    // Weekend bookings are rejected
    QDate saturday=date;
    while(saturday.dayOfWeek()!=Qt::Saturday)saturday=saturday.addDays(1);
    auto weekendSat=availability.validateRequest(lab.id,saturday,QTime(8,30),QTime(10,30));
    QVERIFY(!weekendSat.ok);
    QVERIFY(weekendSat.error.contains("weekend",Qt::CaseInsensitive));
    QVERIFY(!bookings.create(*user,request(lab.id,saturday,QTime(8,30),QTime(10,30))).ok);

    QDate sunday=saturday.addDays(1);
    auto weekendSun=availability.validateRequest(lab.id,sunday,QTime(8,30),QTime(10,30));
    QVERIFY(!weekendSun.ok);
    QVERIFY(weekendSun.error.contains("weekend",Qt::CaseInsensitive));
    QVERIFY(!bookings.create(*user,request(lab.id,sunday,QTime(8,30),QTime(10,30))).ok);
  }
  void configurableScheduleAndClosures(){
    auto lab = addLab("Schedule Lab");
    AvailabilityService availability(*m_db);
    const QDate date = nextWeekday(QDate::currentDate().addDays(3));
    QVERIFY(m_db->setBookingSchedule(QTime(9, 0), QTime(14, 0)).ok);
    const auto configured = m_db->bookingSlots();
    QCOMPARE(configured.size(), 4);
    QCOMPARE(configured.first().start, QTime(9, 0));
    QCOMPARE(configured.last().end, QTime(18, 0));
    QVERIFY(availability.check(lab.id, date, QTime(9, 0), QTime(11, 0)));
    QVERIFY(!availability.check(lab.id, date, QTime(8, 30), QTime(10, 30)));
    QVERIFY(m_db->setClosedDate(date, true).ok);
    QVERIFY(m_db->isClosedDate(date));
    QVERIFY(!availability.check(lab.id, date, QTime(9, 0), QTime(11, 0)));
    QVERIFY(m_db->setClosedDate(date, false).ok);
    QVERIFY(availability.check(lab.id, date, QTime(9, 0), QTime(11, 0)));
  }
  void migratesVersionThreeSchedule() {
    const QString path = m_dir.path() + "/version3-" + QUuid::createUuid().toString(QUuid::WithoutBraces) + ".sqlite";
    {
      DatabaseManager old(path);
      QVERIFY(old.open().ok);
      QSqlQuery query(old.db());
      QVERIFY(query.exec("DROP TABLE booking_schedule"));
      QVERIFY(query.exec("DROP TABLE closed_dates"));
      QVERIFY(query.exec("PRAGMA user_version=3"));
    }
    DatabaseManager migrated(path);
    const auto opened = migrated.open();
    QVERIFY2(opened.ok, qPrintable(opened.error));
    QCOMPARE(migrated.bookingSlots().size(), 4);
    QSqlQuery version(migrated.db());
    QVERIFY(version.exec("PRAGMA user_version"));
    QVERIFY(version.next());
    QCOMPARE(version.value(0).toInt(), 4);
  }
  void bookingOwnershipReviewCancellationAndHistory(){
    auto student=createStudent();auto lab=addLab("L1");BookingService service(*m_db);AvailabilityService availability(*m_db);
    const auto date=nextWeekday(QDate::currentDate().addDays(2));auto r=request(lab.id,date,QTime(8,30),QTime(10,30));QString reference;
    QVERIFY(service.create(*student,r,&reference).ok);QVERIFY(reference.startsWith("LB-"));
    const auto booking=m_db->bookingsForUser(student->id()).first();

    // While pending, slot is blocked (double booking prevented)
    QVERIFY(!service.create(*student,r).ok);
    QVERIFY(!availability.check(lab.id,date,QTime(8,30),QTime(10,30)));

    QVERIFY(!service.review(*student,booking.id,BookingStatus::Approved).ok);
    QVERIFY(!service.cancel(*m_admin,999).ok);
    QVERIFY(service.review(*m_admin,booking.id,BookingStatus::Approved,"Approved").ok);
    QCOMPARE(m_db->notificationsForUser(student->id()).first().action, QString("Booking approved"));

    // While approved, slot is still blocked
    QVERIFY(!service.create(*student,r).ok);

    // When cancelled, the slot is released!
    const auto cancelled=service.cancel(*student,booking.id);QVERIFY2(cancelled.ok,qPrintable(cancelled.error));
    QVERIFY(m_db->bookingById(booking.id)->status==BookingStatus::Cancelled);
    QCOMPARE(m_db->bookingApprovals(booking.id).size(),1);

    // Now slot is available again and can be booked
    QVERIFY(availability.check(lab.id,date,QTime(8,30),QTime(10,30)));
    QString ref2;
    QVERIFY(service.create(*student,r,&ref2).ok);

    // Admin rejects booking -> slot is released again!
    Booking booking2;
    for (const auto& b : m_db->bookingsForUser(student->id())) {
      if (b.status == BookingStatus::Pending) { booking2 = b; break; }
    }
    QVERIFY(booking2.id > 0);
    QVERIFY(service.review(*m_admin, booking2.id, BookingStatus::Rejected, "Rejected").ok);
    QVERIFY(availability.check(lab.id, date, QTime(8, 30), QTime(10, 30)));
  }
  void equipmentReservationsAndRollback(){
    auto user=createStudent();auto lab=addLab("L1");Equipment gear;gear.labId=lab.id;gear.name="Microscope";gear.totalQty=2;gear.availableQty=2;
    QVERIFY(m_db->addEquipment(gear).ok);const auto equipment=m_db->equipment().first();const auto date=nextWeekday(QDate::currentDate().addDays(2));
    BookingRequest first=request(lab.id,date,QTime(8,30),QTime(10,30));first.equipment={{equipment.id,2}};
    QVERIFY(BookingService(*m_db).create(*user,first).ok);
    BookingRequest second=request(lab.id,date,QTime(10,30),QTime(12,30));second.equipment={{equipment.id,1}};
    QVERIFY(BookingService(*m_db).create(*user,second).ok);
    const auto booked=m_db->bookingsForUser(user->id());
    QVERIFY(!booked.isEmpty());
    const auto bookedItems=m_db->equipmentForBooking(booked.first().id);
    QCOMPARE(bookedItems.size(),1);
    QCOMPARE(bookedItems.first().equipmentName,QString("Microscope"));
    QCOMPARE(bookedItems.first().quantity,2);
    BookingRequest conflict=request(lab.id,date,QTime(8,30),QTime(10,30));conflict.equipment={{equipment.id,1}};
    QVERIFY(!BookingService(*m_db).create(*user,conflict).ok);
    const int before=m_db->count("bookings");
    auto broken=request(lab.id,nextWeekday(date.addDays(2)),QTime(8,30),QTime(10,30));broken.equipment={{99999,1}};
    auto result=m_db->transaction([&]{return m_db->insertBooking(broken,"LB-ROLLBACK");});
    QVERIFY(!result.ok);QCOMPARE(m_db->count("bookings"),before);
    auto invalid=gear;invalid.availableQty=3;QVERIFY(!EquipmentManager(*m_db).add(*m_admin,invalid).ok);
  }
  void consistentRulesAcrossRoles(){
    auto student=createStudent();
    auto lecturer=createLecturer();
    auto admin=m_admin;
    auto lab=addLab("ROLE1");
    const auto date=nextWeekday(QDate::currentDate().addDays(3));
    BookingService service(*m_db);

    // All three roles can book an open slot
    QVERIFY(service.create(*student,request(lab.id,date,QTime(8,30),QTime(10,30))).ok);
    QVERIFY(service.create(*lecturer,request(lab.id,date,QTime(10,30),QTime(12,30))).ok);
    QVERIFY(service.create(*admin,request(lab.id,date,QTime(13,30),QTime(15,30))).ok);

    // Double booking rejected consistently for student, lecturer, and administrator
    QVERIFY(!service.create(*student,request(lab.id,date,QTime(8,30),QTime(10,30))).ok);
    QVERIFY(!service.create(*lecturer,request(lab.id,date,QTime(8,30),QTime(10,30))).ok);
    QVERIFY(!service.create(*admin,request(lab.id,date,QTime(8,30),QTime(10,30))).ok);

    // Weekend bookings rejected consistently for student, lecturer, and administrator
    QDate saturday=date;
    while(saturday.dayOfWeek()!=Qt::Saturday)saturday=saturday.addDays(1);
    QVERIFY(!service.create(*student,request(lab.id,saturday,QTime(8,30),QTime(10,30))).ok);
    QVERIFY(!service.create(*lecturer,request(lab.id,saturday,QTime(8,30),QTime(10,30))).ok);
    QVERIFY(!service.create(*admin,request(lab.id,saturday,QTime(8,30),QTime(10,30))).ok);

    // Lunch break bookings rejected consistently for student, lecturer, and administrator
    QVERIFY(!service.create(*student,request(lab.id,date,QTime(12,30),QTime(14,30))).ok);
    QVERIFY(!service.create(*lecturer,request(lab.id,date,QTime(12,30),QTime(14,30))).ok);
    QVERIFY(!service.create(*admin,request(lab.id,date,QTime(12,30),QTime(14,30))).ok);
  }
  void activationAndDeactivation(){
    auto student=createStudent();
    QVERIFY(UserManager(*m_db).deactivate(*m_admin,student->id()).ok);
    QVERIFY(!m_db->userById(student->id())->active);
    QVERIFY(UserManager(*m_db).activate(*m_admin,student->id()).ok);
    QVERIFY(m_db->userById(student->id())->active);

    auto lab=addLab("ACT1");
    QVERIFY(LaboratoryManager(*m_db).deactivate(*m_admin,lab.id).ok);
    bool foundInActive=false;
    for(const auto& l: m_db->laboratories()) if(l.id==lab.id) foundInActive=true;
    QVERIFY(!foundInActive);
    QVERIFY(LaboratoryManager(*m_db).activate(*m_admin,lab.id).ok);

    Equipment gear;gear.labId=lab.id;gear.name="Oscilloscope";gear.totalQty=3;gear.availableQty=3;
    QVERIFY(EquipmentManager(*m_db).add(*m_admin,gear).ok);
    const auto savedGear=m_db->equipment().last();
    QVERIFY(EquipmentManager(*m_db).deactivate(*m_admin,savedGear.id).ok);
    QVERIFY(EquipmentManager(*m_db).activate(*m_admin,savedGear.id).ok);
  }
  void reportServiceGeneratesCountsAndSummary(){
    ReportService reports(*m_db);
    const auto counts=reports.bookingCounts();
    QCOMPARE(counts.total,0);
    const auto labs=reports.laboratoryCounts();
    QVERIFY(labs.contains("Total"));
    const auto html=reports.summaryHtml();
    QVERIFY(html.contains("LabBook report"));
    const QString pdf=m_dir.filePath("report-"+QUuid::createUuid().toString(QUuid::WithoutBraces)+".pdf");
    const auto exported=reports.exportPdf(pdf);
    QVERIFY2(exported.ok,qPrintable(exported.error));
    QFile output(pdf);QVERIFY(output.open(QIODevice::ReadOnly));
    QCOMPARE(output.read(4),QByteArray("%PDF"));
  }
  void managementChangesPersistAcrossConnections(){
    auto student=createStudent();
    auto user=m_db->userById(student->id()).value();
    user.name="Updated Student";
    QVERIFY(UserManager(*m_db).update(*m_admin,user).ok);
    auto lab=addLab("PERSIST");
    lab.name="Updated Laboratory";
    QVERIFY(LaboratoryManager(*m_db).update(*m_admin,lab).ok);
    Equipment item;item.labId=lab.id;item.name="Balance";item.totalQty=2;item.availableQty=2;
    QVERIFY(EquipmentManager(*m_db).add(*m_admin,item).ok);
    item=m_db->equipment().first();item.name="Precision Balance";
    QVERIFY(EquipmentManager(*m_db).update(*m_admin,item).ok);
    DatabaseManager reopened(m_path);
    QVERIFY(reopened.open().ok);
    QCOMPARE(reopened.userById(student->id())->name,QString("Updated Student"));
    QCOMPARE(reopened.laboratories().first().name,QString("Updated Laboratory"));
    QCOMPARE(reopened.equipment().first().name,QString("Precision Balance"));
  }
  void migratesVersionOneWithoutLosingRows(){
    const QString path=m_dir.path()+"/legacy.sqlite";const QString connection="legacy_"+QUuid::createUuid().toString(QUuid::WithoutBraces);
    {
      auto db=QSqlDatabase::addDatabase("QSQLITE",connection);db.setDatabaseName(path);QVERIFY(db.open());QSqlQuery q(db);
      const QStringList sql={
        "CREATE TABLE roles(role_id INTEGER PRIMARY KEY,role_name TEXT UNIQUE NOT NULL)",
        "INSERT INTO roles VALUES(1,'Student'),(2,'Lecturer'),(3,'Administrator')",
        "CREATE TABLE users(user_id INTEGER PRIMARY KEY,eid TEXT UNIQUE NOT NULL,name TEXT NOT NULL,faculty TEXT NOT NULL,department TEXT NOT NULL,contact TEXT NOT NULL,role_id INTEGER NOT NULL,password_hash TEXT NOT NULL,password_salt TEXT NOT NULL,password_iterations INTEGER NOT NULL,active INTEGER NOT NULL,must_change_password INTEGER NOT NULL,created_at TEXT NOT NULL,updated_at TEXT NOT NULL)",
        "INSERT INTO users VALUES(1,'OLD','Old User','','','',3,'hash','salt',210000,1,0,'now','now')",
        "CREATE TABLE laboratories(lab_id INTEGER PRIMARY KEY,lab_name TEXT NOT NULL,lab_number TEXT NOT NULL UNIQUE,location TEXT NOT NULL,department TEXT NOT NULL,capacity INTEGER NOT NULL,description TEXT NOT NULL,status TEXT NOT NULL,active INTEGER NOT NULL,created_at TEXT NOT NULL,updated_at TEXT NOT NULL)",
        "INSERT INTO laboratories VALUES(1,'Lab','L1','','',10,'','Available',1,'now','now')",
        "CREATE TABLE equipment(equipment_id INTEGER PRIMARY KEY,lab_id INTEGER NOT NULL,equipment_name TEXT NOT NULL,total_qty INTEGER NOT NULL,available_qty INTEGER NOT NULL,condition TEXT NOT NULL,status TEXT NOT NULL,active INTEGER NOT NULL,created_at TEXT NOT NULL,updated_at TEXT NOT NULL)",
        "INSERT INTO equipment VALUES(1,1,'Balance',1,1,'Good','Available',1,'now','now')",
        "CREATE TABLE bookings(booking_id INTEGER PRIMARY KEY,reference TEXT NOT NULL UNIQUE,user_id INTEGER NOT NULL,lab_id INTEGER NOT NULL,booking_date TEXT NOT NULL,start_time TEXT NOT NULL,end_time TEXT NOT NULL,purpose TEXT NOT NULL,status TEXT NOT NULL,reviewed_by INTEGER,decision_note TEXT NOT NULL,created_at TEXT NOT NULL,updated_at TEXT NOT NULL)",
        "INSERT INTO bookings VALUES(1,'LB-OLD',1,1,'2099-01-01','09:00','10:00','work','Approved',1,'','now','now')",
        "CREATE TABLE booking_equipment(booking_id INTEGER NOT NULL,equipment_id INTEGER NOT NULL,qty_requested INTEGER NOT NULL,PRIMARY KEY(booking_id,equipment_id))",
        "PRAGMA user_version=1"
      };
      for(const auto&statement:sql)QVERIFY2(q.exec(statement),qPrintable(q.lastError().text()));
      db.close();
    }
    QSqlDatabase::removeDatabase(connection);
    DatabaseManager migrated(path);auto opened=migrated.open();QVERIFY2(opened.ok,qPrintable(opened.error));
    QCOMPARE(migrated.userByEid("OLD")->name,QString("Old User"));
    QCOMPARE(migrated.bookingById(1)->reference,QString("LB-OLD"));
    QCOMPARE(migrated.equipment().first().notes,QString(""));
    QSqlQuery q(migrated.db());QVERIFY(q.exec("PRAGMA user_version"));QVERIFY(q.next());QCOMPARE(q.value(0).toInt(),4);
  }
private:
  std::shared_ptr<User> createStudent(){
    UserRecord student;student.eid="STU"+QUuid::createUuid().toString(QUuid::WithoutBraces).left(6);student.name="Student";student.role=UserRole::Student;
    auto result=AuthService(*m_db).createUser(student,"temporary-password");if(!result.ok)qFatal("%s",qPrintable(result.error));
    auto user=AuthService(*m_db).authenticate(student.eid,"temporary-password");if(!user)qFatal("Could not authenticate test student.");return user;
  }
  std::shared_ptr<User> createLecturer(){
    UserRecord lecturer;lecturer.eid="LEC"+QUuid::createUuid().toString(QUuid::WithoutBraces).left(6);lecturer.name="Lecturer";lecturer.role=UserRole::Lecturer;
    auto result=AuthService(*m_db).createUser(lecturer,"temporary-password");if(!result.ok)qFatal("%s",qPrintable(result.error));
    auto user=AuthService(*m_db).authenticate(lecturer.eid,"temporary-password");if(!user)qFatal("Could not authenticate test lecturer.");return user;
  }
  QDate nextWeekday(QDate from = QDate::currentDate()) const {
    return nextOperatingWeekday(from);
  }
  Laboratory addLab(const QString&number){
    Laboratory lab;lab.name="Laboratory "+number;lab.number=number;lab.capacity=12;
    auto result=m_db->addLaboratory(lab);if(!result.ok)qFatal("%s",qPrintable(result.error));
    for(const auto&entry:m_db->laboratories())if(entry.number==number)return entry;
    qFatal("Inserted laboratory could not be loaded.");return {};
  }
  BookingRequest request(qint64 lab,const QDate&date,const QTime&start,const QTime&end)const{
    BookingRequest r;r.labId=lab;r.date=date;r.start=start;r.end=end;r.purpose="Research work";return r;
  }
  QTemporaryDir m_dir;
  QString m_path;
  std::unique_ptr<DatabaseManager>m_db;
  std::shared_ptr<User>m_admin;
};
QTEST_MAIN(CoreTests)
#include "test_core.moc"
