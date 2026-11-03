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
#include <QCheckBox>
#include <QAbstractItemView>
#include <QScrollArea>
#include <QScrollBar>
#include <QWheelEvent>

class UiTests : public QObject {
  Q_OBJECT
private slots:
  void sidebarScrollsToLastItem() {
    QTemporaryDir directory;
    QVERIFY(directory.isValid());
    DatabaseManager db(directory.filePath("sidebar.sqlite"));
    QVERIFY(db.open().ok);
    UserRecord adminRecord; adminRecord.eid = "ADMIN"; adminRecord.name = "Admin"; adminRecord.role = UserRole::Administrator;
    QVERIFY(AuthService(db).bootstrap(adminRecord, "admin-password").ok);
    auto admin = AuthService(db).authenticate("ADMIN", "admin-password");
    QVERIFY(admin);

    MainWindow window(db, admin);
    window.resize(1000, 480);
    window.show();
    QApplication::processEvents();
    auto* scroll = window.findChild<QScrollArea*>("navScroll");
    auto* header = window.findChild<QFrame*>("brandHeader");
    QVERIFY(scroll && header);
    QVERIFY(scroll->verticalScrollBar()->maximum() > 0);
    const int headerY = header->mapTo(&window, QPoint(0, 0)).y();
    const QPointF position(scroll->viewport()->rect().center());
    QWheelEvent wheel(position, scroll->viewport()->mapToGlobal(position.toPoint()),
                      QPoint(), QPoint(0, -1200), Qt::NoButton, Qt::NoModifier,
                      Qt::NoScrollPhase, false);
    QApplication::sendEvent(scroll->viewport(), &wheel);
    QApplication::processEvents();
    QVERIFY(scroll->verticalScrollBar()->value() > 0);
    scroll->verticalScrollBar()->setValue(scroll->verticalScrollBar()->maximum());
    QApplication::processEvents();
    QCOMPARE(header->mapTo(&window, QPoint(0, 0)).y(), headerY);
    QPushButton* settings = nullptr;
    for (auto* button : scroll->findChildren<QPushButton*>())
      if (button->property("navPage").toString() == "Settings") settings = button;
    QVERIFY(settings);
    QVERIFY(settings->isVisible());
    QVERIFY(scroll->viewport()->rect().contains(settings->mapTo(scroll->viewport(), settings->rect().center())));
    settings->click();
    QCOMPARE(window.findChild<QLabel*>("pageTitle")->text(), QString("Settings"));
  }

  void editingRestoresEquipmentAndResetClearsIt() {
    QTemporaryDir directory;
    QVERIFY(directory.isValid());
    DatabaseManager db(directory.filePath("edit.sqlite"));
    QVERIFY(db.open().ok);
    UserRecord adminRecord; adminRecord.eid = "ADMIN"; adminRecord.name = "Admin"; adminRecord.role = UserRole::Administrator;
    QVERIFY(AuthService(db).bootstrap(adminRecord, "admin-password").ok);
    auto admin = AuthService(db).authenticate("ADMIN", "admin-password");
    QVERIFY(admin);
    if (const QString screenshot = qEnvironmentVariable("LABBOOK_LOGIN_SCREENSHOT"); !screenshot.isEmpty()) {
      LoginWindow login(db);
      login.resize(1240, 800);
      login.show();
      QApplication::processEvents();
      login.grab().save(screenshot);
    }
    UserRecord studentRecord; studentRecord.eid = "STUDENT"; studentRecord.name = "Student";
    QVERIFY(UserManager(db).add(*admin, studentRecord, "student-password").ok);
    auto student = AuthService(db).authenticate("STUDENT", "student-password");
    QVERIFY(student);
    Laboratory lab; lab.name = "Biology"; lab.number = "BIO-1"; lab.capacity = 20;
    QVERIFY(LaboratoryManager(db).add(*admin, lab).ok);
    Equipment item; item.labId = db.laboratories().first().id; item.name = "Microscope"; item.totalQty = 3; item.availableQty = 3;
    QVERIFY(EquipmentManager(db).add(*admin, item).ok);
    BookingRequest request; request.labId = item.labId; request.date = nextOperatingWeekday(QDate::currentDate().addDays(1));
    request.start = QTime(8, 30); request.end = QTime(10, 30); request.purpose = "Practical";
    request.equipment = {{db.equipment().first().id, 2}};
    QVERIFY(BookingService(db).create(*student, request).ok);
    MainWindow window(db, student);
    window.navigateToPage("My Bookings");
    auto* bookings = window.findChild<QTableWidget*>("myBookingsTable");
    QVERIFY(bookings && bookings->rowCount() == 1);
    bookings->setCurrentCell(0, 0);
    QPushButton* edit = nullptr;
    for (auto* button : window.findChildren<QPushButton*>()) if (button->text() == "Edit Pending") edit = button;
    QVERIFY(edit);
    edit->click();
    auto* pages = window.findChild<QStackedWidget*>();
    QVERIFY(pages);
    auto* purpose = pages->currentWidget()->findChild<QComboBox*>("bookingPurposeCombo");
    auto* customPurpose = pages->currentWidget()->findChild<QLineEdit*>("customBookingPurpose");
    QVERIFY(purpose && customPurpose);
    QCOMPARE(purpose->count(), 7);
    QCOMPARE(purpose->currentText(), QString("Other"));
    QCOMPARE(customPurpose->text(), QString("Practical"));
    QVERIFY(!customPurpose->isHidden());
    purpose->setCurrentText("Research Work");
    QVERIFY(customPurpose->isHidden());
    purpose->setCurrentText("Other");
    QVERIFY(!customPurpose->isHidden());
    QCheckBox* microscope = nullptr;
    for (auto* check : pages->currentWidget()->findChildren<QCheckBox*>())
      if (check->text().contains("Microscope")) microscope = check;
    QVERIFY(microscope);
    QVERIFY(microscope->isChecked());
    QPushButton* reset = nullptr;
    for (auto* button : pages->currentWidget()->findChildren<QPushButton*>()) if (button->text() == "Reset Form") reset = button;
    QVERIFY(reset);
    reset->click();
    QCOMPARE(purpose->currentIndex(), 0);
    QVERIFY(customPurpose->isHidden());
    for (auto* check : pages->currentWidget()->findChildren<QCheckBox*>())
      if (check->text().contains("Microscope")) QVERIFY(!check->isChecked());
  }
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
    studentRecord.department = "Science";
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
    QCOMPARE(availabilityDate->buttonSymbols(), QAbstractSpinBox::NoButtons);
    const QDate chosenDate = nextOperatingWeekday(availabilityDate->date().addDays(1));
    QTimer::singleShot(0, [&] {
      auto* dialog = QApplication::activeModalWidget();
      QVERIFY(dialog);
      auto* calendar = dialog->findChild<QCalendarWidget*>("bookingCalendar");
      auto* buttons = dialog->findChild<QDialogButtonBox*>();
      QVERIFY(calendar && buttons);
      calendar->setSelectedDate(chosenDate);
      if (const QString screenshot = qEnvironmentVariable("LABBOOK_CALENDAR_SCREENSHOT"); !screenshot.isEmpty())
        dialog->grab().save(screenshot);
      buttons->button(QDialogButtonBox::Ok)->click();
    });
    calendarButton->click();
    QCOMPARE(availabilityDate->date(), chosenDate);
    QVERIFY(!studentWindow.findChild<QTableWidget*>("adminBookingsTable"));
    studentWindow.navigateToPage("Users");
    QVERIFY(!studentWindow.findChild<QTableWidget*>("usersTable"));

    MainWindow adminWindow(db, admin);
    if (const QString screenshot = qEnvironmentVariable("LABBOOK_ADMIN_SCREENSHOT"); !screenshot.isEmpty()) {
      adminWindow.resize(1280, 840);
      adminWindow.show();
      QApplication::processEvents();
      adminWindow.grab().save(screenshot);
    }
    for (const QString& page : {"Dashboard", "Availability", "All Bookings", "Approvals",
                                "Laboratories", "Equipment", "Users", "Reports",
                                "My Bookings", "New Booking", "Profile", "Updates", "Settings"}) {
      adminWindow.navigateToPage(page);
      if (const QString screenshotDir = qEnvironmentVariable("LABBOOK_PAGES_SCREENSHOT_DIR"); !screenshotDir.isEmpty() &&
          (page == "Availability" || page == "All Bookings" || page == "Settings" || page == "New Booking")) {
        QApplication::processEvents();
        adminWindow.grab().save(screenshotDir + "/" + page.toLower().replace(' ', '-') + ".png");
      }
      auto* title = adminWindow.findChild<QLabel*>("pageTitle");
      QVERIFY(title);
      QCOMPARE(title->text(), page);
    }
    for (const QString& page : {"All Bookings", "Approvals"}) {
      adminWindow.navigateToPage(page);
      auto* currentPage = adminWindow.findChild<QStackedWidget*>()->currentWidget();
      const QString prefix = page == "All Bookings" ? "adminBookings" : "approvals";
      auto* from = currentPage->findChild<QDateEdit*>(prefix + "From");
      auto* to = currentPage->findChild<QDateEdit*>(prefix + "To");
      auto* table = currentPage->findChild<QTableWidget*>(prefix + "Table");
      QVERIFY(from && to && table);
      QCOMPARE(from->date(), QDate::currentDate());
      QCOMPARE(to->date(), QDate::currentDate());
      QCOMPARE(from->displayFormat(), QString("dd/MM/yyyy"));
      QVERIFY(table->rowCount() > 0);
      QVERIFY(table->isRowHidden(0));
      to->setDate(request.date);
      QApplication::processEvents();
      QVERIFY(!table->isRowHidden(0));
    }
    adminWindow.resize(900, 560);
    adminWindow.show();
    adminWindow.navigateToPage("Settings");
    QApplication::processEvents();
    auto* settingsView = adminWindow.findChild<QStackedWidget*>()->currentWidget();
    auto* settingsScroll = settingsView->findChild<QScrollArea*>();
    QVERIFY(settingsScroll);
    QVERIFY(settingsScroll->verticalScrollBar()->maximum() > 0);
    QPushButton* passwordButton = nullptr;
    for (auto* button : settingsView->findChildren<QPushButton*>())
      if (button->text() == "Update Account Password") passwordButton = button;
    QVERIFY(passwordButton);
    settingsScroll->ensureWidgetVisible(passwordButton);
    QApplication::processEvents();
    QVERIFY(settingsScroll->viewport()->rect().contains(
      passwordButton->mapTo(settingsScroll->viewport(), passwordButton->rect().center())));
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
      auto* capacity = dialog->findChild<QSpinBox*>();
      auto* status = dialog->findChild<QComboBox*>();
      if (capacity && status) {
        capacity->setValue(24);
        QTest::mouseClick(capacity, Qt::LeftButton, {}, QPoint(capacity->width() - 13, 9));
        facilityDialogChecked &= capacity->value() == 25;
        QTest::mouseClick(capacity, Qt::LeftButton, {}, QPoint(capacity->width() - 13, capacity->height() - 9));
        facilityDialogChecked &= capacity->value() == 24;
        capacity->findChild<QLineEdit*>()->setText("31");
        capacity->interpretText();
        facilityDialogChecked &= capacity->value() == 31;
        QTest::mouseClick(status, Qt::LeftButton, {}, QPoint(status->width() - 13, status->height() / 2));
        facilityDialogChecked &= status->view()->isVisible();
        status->hidePopup();
        status->setCurrentIndex(1);
        facilityDialogChecked &= status->currentText() == "Unavailable";
      }
      const QString screenshot = qEnvironmentVariable("LABBOOK_FACILITY_SCREENSHOT");
      if (!screenshot.isEmpty()) dialog->grab().save(screenshot);
      dialog->reject();
    });
    editFacility->click();
    QVERIFY(facilityDialogChecked);
    adminWindow.navigateToPage("Equipment");
    auto* equipmentPages = adminWindow.findChild<QStackedWidget*>();
    QPushButton* addEquipment = nullptr;
    for (auto* button : equipmentPages->currentWidget()->findChildren<QPushButton*>())
      if (button->text() == "Add Equipment") addEquipment = button;
    QVERIFY(addEquipment);
    bool equipmentControlsChecked = false;
    QTimer::singleShot(150, [&] {
      auto* dialog = QApplication::activeModalWidget();
      if (!dialog) return;
      const auto spins = dialog->findChildren<QSpinBox*>();
      const auto combos = dialog->findChildren<QComboBox*>();
      equipmentControlsChecked = spins.size() == 2 && combos.size() == 2;
      if (equipmentControlsChecked) {
        spins[0]->setValue(3);
        QTest::mouseClick(spins[0], Qt::LeftButton, {}, QPoint(spins[0]->width() - 13, 9));
        equipmentControlsChecked &= spins[0]->value() == 4;
        QTest::mouseClick(spins[0], Qt::LeftButton, {}, QPoint(spins[0]->width() - 13, spins[0]->height() - 9));
        equipmentControlsChecked &= spins[0]->value() == 3;
        QTest::mouseClick(combos[0], Qt::LeftButton, {}, QPoint(combos[0]->width() - 13, combos[0]->height() / 2));
        equipmentControlsChecked &= combos[0]->view()->isVisible();
        combos[0]->hidePopup();
        combos[1]->setCurrentIndex(1);
        equipmentControlsChecked &= combos[1]->currentText() == "Unavailable";
      }
      const QString screenshot = qEnvironmentVariable("LABBOOK_EQUIPMENT_SCREENSHOT");
      if (!screenshot.isEmpty()) dialog->grab().save(screenshot);
      dialog->close();
    });
    addEquipment->click();
    QVERIFY(equipmentControlsChecked);
    adminWindow.navigateToPage("All Bookings");
    auto* table = adminWindow.findChild<QTableWidget*>("adminBookingsTable");
    auto* search = adminWindow.findChild<QLineEdit*>("adminBookingsSearch");
    QVERIFY(table && search);
    QCOMPARE(table->rowCount(), 1);
    search->setText("no matching reservation");
    QVERIFY(table->isRowHidden(0));
    search->setText("Physics");
    QVERIFY(!table->isRowHidden(0));
    auto* statusFilter = adminWindow.findChild<QComboBox*>("adminBookingsStatus");
    QVERIFY(statusFilter);
    statusFilter->setCurrentText("Approved");
    QVERIFY(table->isRowHidden(0));
    statusFilter->setCurrentText("Pending");
    QVERIFY(!table->isRowHidden(0));
    auto* departmentFilter = adminWindow.findChild<QLineEdit*>("adminBookingsDepartment");
    QVERIFY(departmentFilter);
    departmentFilter->setText("Engineering");
    QVERIFY(table->isRowHidden(0));
    departmentFilter->setText("Science");
    QVERIFY(!table->isRowHidden(0));
    adminWindow.navigateToPage("Approvals");
    auto* approvalTable = adminWindow.findChild<QTableWidget*>("approvalsTable");
    auto* approvalDepartment = adminWindow.findChild<QLineEdit*>("approvalsDepartment");
    QVERIFY(approvalTable && approvalDepartment && approvalTable->rowCount() == 1);
    approvalDepartment->setText("Engineering");
    QVERIFY(approvalTable->isRowHidden(0));
    approvalDepartment->setText("Science");
    QVERIFY(!approvalTable->isRowHidden(0));

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
