#pragma once
#include "domain.h"
#include <QSqlDatabase>
#include <QVector>
#include <functional>

class DatabaseManager {
public:
  explicit DatabaseManager(QString path = {});
  ~DatabaseManager();
  ServiceResult open();
  QSqlDatabase db() const { return m_db; }
  ServiceResult transaction(const std::function<ServiceResult()>& fn);
  bool hasUsers() const;
  QVector<UserRecord> users(QString filter = {}, bool includeInactive = true) const;
  std::optional<UserRecord> userByEid(const QString& eid) const;
  std::optional<UserRecord> userById(qint64 id) const;
  ServiceResult insertUser(const UserRecord&, const QString& hash, const QString& salt, int iterations, bool mustChange, bool approved=true);
  ServiceResult updateUser(const UserRecord&);
  ServiceResult setPassword(qint64 id,const QString& hash,const QString& salt,int iterations,bool mustChange);
  ServiceResult setUserActive(qint64 id,bool active);
  ServiceResult setUserApproved(qint64 id,bool approved);
  QVector<Laboratory> laboratories(bool includeInactive=false) const;
  ServiceResult addLaboratory(const Laboratory&);
  ServiceResult updateLaboratory(const Laboratory&);
  ServiceResult setLaboratoryActive(qint64 id,bool active);
  QVector<Equipment> equipment(bool includeInactive=false) const;
  ServiceResult addEquipment(const Equipment&);
  ServiceResult updateEquipment(const Equipment&);
  ServiceResult setEquipmentActive(qint64 id,bool active);
  QVector<Booking> bookingsForUser(qint64 userId) const;
  QVector<Booking> allBookings(bool pendingOnly=false) const;
  std::optional<Booking> bookingById(qint64 id) const;
  QVector<BookingApproval> bookingApprovals(qint64 bookingId) const;
  QVector<BookedEquipment> equipmentForBooking(qint64 bookingId) const;
  bool isAvailable(qint64 labId,const QDate& date,const QTime& start,const QTime& end,qint64 exclude=-1) const;
  ServiceResult checkEquipmentAvailability(qint64 labId,const QDate&,const QTime&,const QTime&,const QVector<QPair<qint64,int>>&,qint64 exclude=-1) const;
  ServiceResult insertBooking(const BookingRequest&, const QString& reference, qint64* idOut=nullptr);
  ServiceResult updateBooking(const Booking&, const BookingRequest&);
  ServiceResult setBookingStatus(qint64 id, BookingStatus status, qint64 reviewer, const QString& note);
  ServiceResult cancelBooking(qint64 id, qint64 actorId, const QString& note={});
  int count(const QString& table, const QString& where = {}) const;
private:
  QString m_path;
  QSqlDatabase m_db;
  ServiceResult migrate();
  ServiceResult execute(const QString& sql) const;
};
