#pragma once
#include "database.h"
#include <QCryptographicHash>
#include <QRandomGenerator>

bool hasPermission(const User& user,Permission permission);

class AuthService {
public:
  explicit AuthService(DatabaseManager& db):m_db(db){}
  bool needsBootstrap() const{return !m_db.hasUsers();}
  ServiceResult bootstrap(const UserRecord&,const QString&password);
  ServiceResult createUser(const UserRecord&,const QString&temporaryPassword);
  ServiceResult registerAccount(const UserRecord&,const QString&password);
  std::shared_ptr<User> authenticate(const QString& eid,const QString&password,QString*error=nullptr) const;
  ServiceResult changePassword(qint64 id,const QString&oldPassword,const QString&newPassword);
  ServiceResult resetPasswordByAdmin(const User&actor,qint64 userId,const QString&temporaryPassword);
private:
  DatabaseManager&m_db;
  static QString salt();
  static QString derive(const QString&,const QString&,int);
  static bool verify(const QString&,const QString&,const QString&,int);
};

class UserManager {
public:
  explicit UserManager(DatabaseManager&db):m_db(db){}
  ServiceResult add(const User&actor,const UserRecord&,const QString&temporaryPassword);
  ServiceResult update(const User&actor,const UserRecord&);
  ServiceResult deactivate(const User&actor,qint64 userId);
  ServiceResult activate(const User&actor,qint64 userId);
  ServiceResult approve(const User&actor,qint64 userId);
  QVector<UserRecord> search(const User&actor,const QString&query) const;
private: DatabaseManager&m_db;
};
class LaboratoryManager {
public:
  explicit LaboratoryManager(DatabaseManager&db):m_db(db){}
  ServiceResult add(const User&actor,const Laboratory&);
  ServiceResult update(const User&actor,const Laboratory&);
  ServiceResult deactivate(const User&actor,qint64 id);
  ServiceResult activate(const User&actor,qint64 id);
  QVector<Laboratory> search(const User&actor,const QString&query) const;
private: DatabaseManager&m_db;
};
class AvailabilityService {
public:
  explicit AvailabilityService(DatabaseManager&db):m_db(db){}
  bool check(qint64 lab,const QDate&date,const QTime&start,const QTime&end,qint64 exclude=-1) const;
  ServiceResult validateRequest(qint64 lab,const QDate&date,const QTime&start,const QTime&end,qint64 exclude=-1) const;
  QVector<Laboratory> available(const QDate&,const QTime&,const QTime&) const;
private: DatabaseManager&m_db;
};
class BookingService {
public:
  explicit BookingService(DatabaseManager&db):m_db(db),m_availability(db){}
  ServiceResult create(const User&actor,BookingRequest,QString*reference=nullptr);
  ServiceResult update(const User&actor,qint64 bookingId,BookingRequest);
  ServiceResult cancel(const User&actor,qint64 bookingId,const QString&note={});
  ServiceResult review(const User&actor,qint64 bookingId,BookingStatus,const QString&note={});
private:
  DatabaseManager&m_db;
  AvailabilityService m_availability;
};
class EquipmentManager {
public:
  explicit EquipmentManager(DatabaseManager&db):m_db(db){}
  ServiceResult add(const User&actor,const Equipment&);
  ServiceResult update(const User&actor,const Equipment&);
  ServiceResult deactivate(const User&actor,qint64 id);
  ServiceResult activate(const User&actor,qint64 id);
  QVector<Equipment> search(const User&actor,const QString&query) const;
private: DatabaseManager&m_db;
};

struct ReportCounts {
  int total{},pending{},approved{},rejected{},cancelled{},completed{};
};
class ReportService {
public:
  explicit ReportService(DatabaseManager& db):m_db(db){}
  ReportCounts bookingCounts(const QDate&from={},const QDate&to={}) const;
  QHash<QString,int> laboratoryCounts() const;
  QHash<QString,int> equipmentCounts() const;
  QString summaryHtml(const QDate&from={},const QDate&to={}) const;
  ServiceResult exportPdf(const QString&filePath,const QDate&from={},const QDate&to={}) const;
private: DatabaseManager&m_db;
};
