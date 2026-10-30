#include "mainwindow.h"
#include <QtTest>
#include <QTemporaryDir>
#include <QPushButton>
#include <QLineEdit>
#include <QTableWidget>
#include <QComboBox>
#include <QStackedWidget>
#include <QCalendarWidget>
#include <QDateEdit>
#include <QDialogButtonBox>
#include <QTimer>
#include <QSpinBox>
#include <QDialog>

class UiTests : public QObject {
  Q_OBJECT
private slots:
  void navigationAndSearchRespectRoles() {
    QTemporaryDir directory;
    QVERIFY(directory.isValid());
    DatabaseManager db(directory.filePath("ui.sqlite"));
    QVERIFY(db.open().ok);
    UserRecord adminRecord;
    adminRecord.eid = "ADMIN";
    adminRecord.name = "Campus Administrator";
    adminRecord.role = UserRole::Administrator;
    QVERIFY(AuthService(db).bootstrap(adminRecord, "admin-password").ok);
    auto admin = AuthService(db).authenticate("ADMIN", "admin-password");
    QVERIFY(admin);

    UserRecord studentRecord;
    studentRecord.eid = "STUDENT";
    studentRecord.name = "Student One";
    studentRecord.role = UserRole::Student;
    QVERIFY(UserManager(db).add(*admin, studentRecord, "student-password").ok);
    auto student = AuthService(db).authenticate("STUDENT", "student-password");
    QVERIFY(student);

    Laboratory lab;
    lab.name = "Physics Laboratory";
    lab.number = "PHY-01";
    lab.capacity = 24;
    QVERIFY(LaboratoryManager(db).add(*admin, lab).ok);
    const auto savedLab = db.laboratories().first();
    BookingRequest request;
    request.labId = savedLab.id;
    request.date = nextOperatingWeekday(QDate::currentDate().addDays(1));
    request.start = QTime(8, 30);
    request.end = QTime(10, 30);
    request.purpose = "Physics practical";
    QVERIFY(BookingService(db).create(*student, request).ok);

    MainWindow studentWindow(db, student);
    QVERIFY(studentWindow.findChildren<QPushButton*>("navButtonActive").size() == 1);
    studentWindow.navigateToPage("My Bookings");
    auto* studentPages = studentWindow.findChild<QStackedWidget*>();
    QVERIFY(studentPages);
    QVERIFY(studentPages->currentWidget()->findChild<QTableWidget*>("myBookingsTable"));
    QVERIFY(!studentPages->currentWidget()->findChild<QComboBox*>());
    auto* bookingTable = studentWindow.findChild<QTableWidget*>("myBookingsTable");
    QVERIFY(bookingTable);
    QCOMPARE(bookingTable->rowCount(), 1);
    bool foundCount = false;
    for (auto* button : studentWindow.findChildren<QPushButton*>())
      foundCount |= button->property("navPage").toString() == "My Bookings" && button->text() == "My Bookings (1)";
    QVERIFY(foundCount);
    studentWindow.navigateToPage("New Booking");
    QVERIFY(studentPages->currentWidget()->findChild<QComboBox*>());
    QVERIFY(!studentPages->currentWidget()->findChild<QTableWidget*>("myBookingsTable"));
    studentWindow.navigateToPage("Availability");
    auto* availabilityDate = studentPages->currentWidget()->findChild<QDateEdit*>();
    auto* calendarButton = studentPages->currentWidget()->findChild<QPushButton*>("calendarPickerButton");
    QVERIFY(availabilityDate && calendarButton);
    const QDate chosenDate = nextOperatingWeekday(availabilityDate->date().addDays(1));
    QTimer::singleShot(0, [&] {
      auto* dialog = QApplication::activeModalWidget();
      QVERIFY(dialog);
      auto* calendar = dialog->findChild<QCalendarWidget*>("bookingCalendar");
      auto* buttons = dialog->findChild<QDialogButtonBox*>();
      QVERIFY(calendar && buttons);
      calendar->setSelectedDate(chosenDate);
      buttons->button(QDialogButtonBox::Ok)->click();
    });
    calendarButton->click();
    QCOMPARE(availabilityDate->date(), chosenDate);
    QVERIFY(!studentWindow.findChild<QTableWidget*>("adminBookingsTable"));
    studentWindow.navigateToPage("Users");
    QVERIFY(!studentWindow.findChild<QTableWidget*>("usersTable"));

    MainWindow adminWindow(db, admin);
    for (const QString& page : {"Dashboard", "Availability", "All Bookings", "Approvals",
                                "Laboratories", "Equipment", "Users", "Reports",
                                "My Bookings", "New Booking", "Profile", "Settings"}) {
      adminWindow.navigateToPage(page);
      auto* title = adminWindow.findChild<QLabel*>("pageTitle");
      QVERIFY(title);
      QCOMPARE(title->text(), page);
    }
    adminWindow.navigateToPage("Laboratories");
    auto* labTable = adminWindow.findChild<QTableWidget*>("laboratoriesTable");
    QVERIFY(labTable && labTable->rowCount() == 1);
    labTable->setCurrentCell(0, 0);
    QPushButton* editFacility = nullptr;
    for (auto* button : adminWindow.findChildren<QPushButton*>())
      if (button->text() == "Edit Selected" && button->parentWidget() == labTable->parentWidget())
        editFacility = button;
    // Locate the action through the visible page if its layout owns the button.
    if (!editFacility) {
      auto* pages = adminWindow.findChild<QStackedWidget*>();
      for (auto* button : pages->currentWidget()->findChildren<QPushButton*>())
        if (button->text() == "Edit Selected") { editFacility = button; break; }
    }
    QVERIFY(editFacility);
    bool facilityDialogChecked = false;
    QTimer::singleShot(150, [&] {
      auto* dialog = adminWindow.findChild<QDialog*>("facilityDialog");
      if (!dialog) return;
      facilityDialogChecked = dialog->findChildren<QLineEdit*>().size() >= 5
          && dialog->findChild<QSpinBox*>() && dialog->findChild<QComboBox*>()
          && dialog->findChild<QPushButton*>("facilityCancelButton");
      const QString screenshot = qEnvironmentVariable("LABBOOK_FACILITY_SCREENSHOT");
      if (!screenshot.isEmpty()) dialog->grab().save(screenshot);
      dialog->reject();
    });
    editFacility->click();
    QVERIFY(facilityDialogChecked);
    adminWindow.navigateToPage("All Bookings");
    auto* table = adminWindow.findChild<QTableWidget*>("adminBookingsTable");
    auto* search = adminWindow.findChild<QLineEdit*>("adminBookingsSearch");
    QVERIFY(table && search);
    QCOMPARE(table->rowCount(), 1);
    search->setText("no matching reservation");
    QVERIFY(table->isRowHidden(0));
    search->setText("Physics");
    QVERIFY(!table->isRowHidden(0));

    auto* collapse = adminWindow.findChild<QPushButton*>("collapseNav");
    QVERIFY(collapse);
    collapse->click();
    QCOMPARE(collapse->toolTip(), QString("Expand navigation"));
    collapse->click();
    QCOMPARE(collapse->toolTip(), QString("Collapse navigation"));
  }
};

QTEST_MAIN(UiTests)
#include "test_ui.moc"
