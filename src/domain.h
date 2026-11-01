#pragma once
#include <QString>
#include <QDate>
#include <QTime>
#include <QVector>
#include <memory>
#include <optional>
#include <utility>

enum class UserRole { Student, Lecturer, Administrator };
enum class LaboratoryStatus { Available, Unavailable, UnderMaintenance };
enum class EquipmentStatus { Available, Unavailable, Maintenance };
enum class BookingStatus { Pending, Approved, Rejected, Cancelled, Completed };
enum class Permission { ViewDashboard, CreateBooking, ManageOwnBooking, ManageUsers, ManageLabs, ManageEquipment, ReviewBookings, ViewReports };

QString toString(UserRole);
QString toString(LaboratoryStatus);
QString toString(EquipmentStatus);
QString toString(BookingStatus);
UserRole userRoleFromString(const QString&);
BookingStatus bookingStatusFromString(const QString&);

class User {
public:
  User(qint64 id, QString eid, QString name, UserRole role, bool active=true, bool mustChange=false);
  virtual ~User() = default;
  qint64 id() const;
  QString eid() const;
  QString name() const;
  UserRole role() const;
  bool active() const;
  bool mustChangePassword() const;
  virtual QString roleName() const = 0;
  virtual QVector<Permission> permissions() const = 0;
protected:
  qint64 m_id;
  QString m_eid, m_name;
  UserRole m_role;
  bool m_active, m_mustChange;
};
class Student final : public User {
public: using User::User; QString roleName() const override; QVector<Permission> permissions() const override;
};
class Lecturer final : public User {
public: using User::User; QString roleName() const override; QVector<Permission> permissions() const override;
};
class Administrator final : public User {
public: using User::User; QString roleName() const override; QVector<Permission> permissions() const override;
};

struct UserRecord {
  qint64 id{};
  QString eid,name,email,faculty,department,contact;
  UserRole role{UserRole::Student};
  bool active{true},mustChange{false},approved{true};
};
struct Laboratory {
  qint64 id{};
  QString name,number,location,department,description;
  int capacity{};
  LaboratoryStatus status{LaboratoryStatus::Available};
  bool active{true};
};
struct Equipment {
  qint64 id{};
  qint64 labId{};
  QString name,condition,notes;
  int totalQty{},availableQty{};
  EquipmentStatus status{EquipmentStatus::Available};
  bool active{true};
};
struct Booking {
  qint64 id{};
  QString reference,purpose,decisionNote;
  qint64 userId{},labId{};
  QDate date;
  QTime start,end;
  BookingStatus status{BookingStatus::Pending};
};
struct BookingRequest {
  qint64 userId{},labId{};
  QDate date;
  QTime start,end;
  QString purpose;
  QVector<QPair<qint64,int>> equipment;
};
struct BookedEquipment {
  qint64 equipmentId{};
  QString equipmentName;
  int quantity{};
};
struct BookingApproval {
  qint64 id{},bookingId{},adminId{};
  QString action,comment,adminName;
  QString actionTime;
};
struct AppNotification {
  qint64 id{}, bookingId{};
  QString action, description, timestamp;
};
struct FixedBookingSlot {
  int id{};
  QTime start;
  QTime end;
  QString label;      // e.g. "8:30 AM – 10:30 AM"
  QString fullTitle;  // e.g. "Slot 1 (8:30 AM – 10:30 AM)"
};

const QVector<FixedBookingSlot>& fixedBookingSlots();
std::optional<FixedBookingSlot> findFixedBookingSlot(const QTime& start, const QTime& end);
bool isFixedBookingSlot(const QTime& start, const QTime& end);
bool isWeekend(const QDate& date);
bool overlapsLunchBreak(const QTime& start, const QTime& end);
QDate nextOperatingWeekday(const QDate& from = QDate::currentDate());

struct ServiceResult {
  bool ok{true};
  QString error;
  static ServiceResult success(){return {};}
  static ServiceResult failure(QString e){return {false,std::move(e)};}
};
