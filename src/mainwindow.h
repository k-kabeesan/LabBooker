#pragma once
#include "services.h"
#include <QMainWindow>
#include <QStackedWidget>
#include <QLabel>
#include <QVector>
#include <QPair>
#include <QPixmap>

class QLineEdit;
class QTableWidget;
class QCheckBox;
class QPushButton;
class QComboBox;
class QDateEdit;
class QTimeEdit;
class QResizeEvent;

class MainWindow final : public QMainWindow {
  Q_OBJECT
public:
  MainWindow(DatabaseManager&,std::shared_ptr<User>,QWidget*parent=nullptr);
signals:
  void logoutRequested();
public slots:
  void navigateToPage(const QString& pageTitle);
  void prefillBookingForm(qint64 labId, const QDate& date, const QTime& start, const QTime& end);
private:
  DatabaseManager& m_db;
  std::shared_ptr<User> m_user;
  QStackedWidget* m_pages{};
  QWidget* m_myBookingsPage{};
  QLabel* m_title{};
  QLabel* m_subtitle{};
  QList<QPushButton*> m_navButtons;

  void addPage(const QString&, QWidget*);
  QWidget* dashboard();
  QWidget* bookings();
  QWidget* availability();
  QWidget* approvals();
  QWidget* adminBookings();
  QWidget* laboratoriesPage();
  QWidget* equipmentPage();
  QWidget* usersPage();
  QWidget* reportsPage();
  QWidget* profilePage();
  QWidget* settingsPage();
  QWidget* updatesPage();
  QWidget* simpleTable(const QString&, const QStringList&);

  void applyTheme(const QString& theme);
  void refreshDashboard();
  void refreshCurrentPage();
  void updateNavBadges();

  void addLaboratoryDialog();
  void editLaboratoryDialog(qint64 id = 0);
  void addUserDialog();
  void editUserDialog(qint64 id = 0);
  void addEquipmentDialog();
  void editEquipmentDialog(qint64 id = 0);

  void refreshLaboratoryCombos();
  void showBookingDetails(qint64 bookingId);

  // Cross-page widgets & combos
  QComboBox* m_availabilityLabCombo{};
  QComboBox* m_availabilitySlotCombo{};
  QDateEdit* m_availabilityDateEdit{};

  QComboBox* m_bookingLabCombo{};
  QDateEdit* m_bookingDateEdit{};
  QTimeEdit* m_bookingStartEdit{};
  QTimeEdit* m_bookingEndEdit{};
  std::function<void()> m_refreshEquipmentFn;
  std::function<void()> m_refreshBookingSlotsFn;

  QVector<QPushButton*> m_bookingSlotButtons;
  QLabel* m_bookingSlotNotice{};
  int m_selectedSlotId{1};
};

class LoginWindow final : public QWidget {
  Q_OBJECT
public:
  LoginWindow(DatabaseManager&, QWidget* parent = nullptr);
  void reset();
protected:
  void resizeEvent(QResizeEvent* event) override;
private:
  DatabaseManager& m_db;
  QLineEdit* m_eid{};
  QLineEdit* m_password{};
  QCheckBox* m_remember{};
  QLabel* m_status{};
  QLabel* m_backgroundLabel{};
  QPixmap m_loginBackground;
  void login();
  void requestAccount();
signals:
  void authenticated(std::shared_ptr<User>);
};
