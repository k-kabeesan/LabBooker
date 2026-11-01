#include "mainwindow.h"
#include <QApplication>
#include <QFormLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QTableWidget>
#include <QHeaderView>
#include <QMessageBox>
#include <QDateEdit>
#include <QCalendarWidget>
#include <QDialogButtonBox>
#include <QTimeEdit>
#include <QTextDocument>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QComboBox>
#include <QFileDialog>
#include <QRegularExpression>
#include <QDir>
#include <QStandardPaths>
#include <QPainterPath>
#include <QLinearGradient>
#include <QPainter>
#include <QPixmap>
#include <QIcon>
#include <QFileInfo>
#include <QFile>
#include <QFrame>
#include <QDialog>
#include <QSpinBox>
#include <QHash>
#include <QCheckBox>
#include <QSettings>
#include <QGraphicsDropShadowEffect>
#include <QScrollArea>
#include <QInputDialog>
#include <QPlainTextEdit>
#include <QTextEdit>
#include <QTextStream>
#include <QDateTime>
#include <QSet>
#include <algorithm>
#include <memory>

namespace {

QPushButton* createButton(const QString& text, const QString& styleClass = QString()) {
  auto* b = new QPushButton(text);
  b->setMinimumHeight(36);
  b->setCursor(Qt::PointingHandCursor);
  if (!styleClass.isEmpty()) b->setProperty("btnStyle", styleClass);
  return b;
}

QString userInitials(const QString& name) {
  const auto parts = name.trimmed().split(QRegularExpression("\\s+"), Qt::SkipEmptyParts);
  if (parts.isEmpty()) return QStringLiteral("U");
  QString result = parts.first().left(1).toUpper();
  if (parts.size() > 1) result += parts.last().left(1).toUpper();
  else if (parts.first().size() > 1) result += parts.first().mid(1, 1).toUpper();
  return result;
}

QIcon navigationIcon(const QString& name, const QColor& color) {
  QPixmap pixmap(24, 24);
  pixmap.fill(Qt::transparent);
  QPainter p(&pixmap);
  p.setRenderHint(QPainter::Antialiasing);
  p.setPen(QPen(color, 2, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
  p.setBrush(Qt::NoBrush);

  if (name == "dashboard") {
    p.drawRoundedRect(3, 3, 7, 7, 2, 2);
    p.drawRoundedRect(14, 3, 7, 7, 2, 2);
    p.drawRoundedRect(3, 14, 7, 7, 2, 2);
    p.drawRoundedRect(14, 14, 7, 7, 2, 2);
  } else if (name == "availability") {
    p.drawRoundedRect(3, 5, 18, 16, 2, 2);
    p.drawLine(3, 10, 21, 10);
    p.drawLine(8, 2, 8, 6);
    p.drawLine(16, 2, 16, 6);
    p.drawLine(10, 15, 14, 15);
  } else if (name == "laboratories") {
    QPainterPath flask;
    flask.moveTo(9, 4); flask.lineTo(15, 4);
    flask.lineTo(14, 8); flask.lineTo(14, 12);
    flask.lineTo(20, 20); flask.quadTo(21, 22, 18, 22);
    flask.lineTo(6, 22); flask.quadTo(3, 22, 4, 20);
    flask.lineTo(10, 12); flask.lineTo(10, 8);
    flask.closeSubpath();
    p.drawPath(flask);
    p.drawLine(7, 18, 17, 18);
  } else if (name == "allbookings") {
    p.drawRoundedRect(4, 3, 16, 18, 2, 2);
    p.drawLine(8, 8, 16, 8);
    p.drawLine(8, 12, 16, 12);
    p.drawLine(8, 16, 13, 16);
  } else if (name == "approvals") {
    p.drawEllipse(3, 3, 18, 18);
    QPainterPath check;
    check.moveTo(7, 12); check.lineTo(10.5, 15.5); check.lineTo(17, 8.5);
    p.drawPath(check);
  } else if (name == "users") {
    p.drawEllipse(8, 3, 8, 8);
    p.drawArc(4, 13, 16, 10, 0, 180 * 16);
    p.drawArc(1, 8, 6, 6, 20 * 16, 145 * 16);
    p.drawArc(17, 8, 6, 6, 15 * 16, 145 * 16);
  } else if (name == "equipment") {
    QPainterPath tool;
    tool.moveTo(5, 19); tool.lineTo(13, 11);
    tool.moveTo(13, 11); tool.cubicTo(10, 8, 12, 4, 16, 4);
    tool.lineTo(18, 4); tool.lineTo(16, 7); tool.lineTo(19, 10);
    tool.lineTo(22, 8); tool.lineTo(22, 10); tool.cubicTo(22, 14, 18, 16, 15, 13);
    tool.lineTo(7, 21); tool.cubicTo(4, 23, 2, 21, 5, 19);
    p.drawPath(tool);
  } else if (name == "bookings") {
    p.drawRoundedRect(5, 2.5, 14, 19, 2, 2);
    p.drawLine(8, 7, 16, 7);
    p.drawLine(8, 11, 16, 11);
    p.drawLine(8, 15, 16, 15);
  } else if (name == "reports") {
    p.drawLine(4, 20, 4, 13);
    p.drawLine(10, 20, 10, 7);
    p.drawLine(16, 20, 16, 10);
    p.drawLine(22, 20, 22, 4);
    p.drawLine(2, 21, 22, 21);
  } else if (name == "profile") {
    p.drawEllipse(8, 3, 8, 8);
    p.drawArc(3, 13, 18, 10, 0, 180 * 16);
  } else if (name == "settings") {
    p.drawEllipse(8, 8, 8, 8);
    p.drawEllipse(10.5, 10.5, 3, 3);
    p.drawLine(12, 2, 12, 6); p.drawLine(12, 18, 12, 22);
    p.drawLine(2, 12, 6, 12); p.drawLine(18, 12, 22, 12);
  } else if (name == "logout") {
    p.drawRoundedRect(3, 3, 11, 18, 2, 2);
    p.drawLine(10, 12, 21, 12);
    p.drawLine(17, 8, 21, 12);
    p.drawLine(17, 16, 21, 12);
  }
  p.end();
  return QIcon(pixmap);
}

QPixmap flaskMark() {
  QPixmap icon(34, 34);
  icon.fill(Qt::transparent);
  QPainter p(&icon);
  p.setRenderHint(QPainter::Antialiasing);
  p.setPen(QPen(Qt::white, 2.5, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
  p.setBrush(Qt::NoBrush);
  QPainterPath shape;
  shape.moveTo(11, 4); shape.lineTo(23, 4);
  shape.lineTo(21, 8); shape.lineTo(21, 13);
  shape.lineTo(28, 26); shape.quadTo(29, 29, 25, 29);
  shape.lineTo(9, 29); shape.quadTo(5, 29, 6, 26);
  shape.lineTo(13, 13); shape.lineTo(13, 8);
  shape.closeSubpath();
  p.drawPath(shape);
  p.drawLine(10, 22, 24, 22);
  p.end();
  return icon;
}

QWidget* createStatusBadgeWidget(const QString& text, const QString& type) {
  auto* w = new QWidget;
  auto* l = new QHBoxLayout(w);
  l->setContentsMargins(6, 4, 6, 4);
  l->setAlignment(Qt::AlignCenter);
  auto* badge = new QLabel(text);
  badge->setAlignment(Qt::AlignCenter);
  QString bg = "#f1f5f9", fg = "#475569", border = "#e2e8f0";
  if (type == "Approved" || type == "Available" || type == "Active" || type == "Yes") {
    bg = "#d1fae5"; fg = "#065f46"; border = "#a7f3d0";
  } else if (type == "Pending" || type == "Under Maintenance" || type == "Maintenance" || type == "Pending approval") {
    bg = "#fef3c7"; fg = "#92400e"; border = "#fde68a";
  } else if (type == "Rejected" || type == "Unavailable" || type == "Inactive" || type == "No") {
    bg = "#fee2e2"; fg = "#991b1b"; border = "#fca5a5";
  } else if (type == "Completed") {
    bg = "#dbeafe"; fg = "#1e40af"; border = "#bfdbfe";
  } else if (type == "Cancelled") {
    bg = "#f1f5f9"; fg = "#475569"; border = "#cbd5e1";
  }
  badge->setStyleSheet(QString("background:%1; color:%2; border:1px solid %3; border-radius:12px; padding:3px 10px; font-weight:600; font-size:11px;").arg(bg, fg, border));
  l->addWidget(badge);
  return w;
}

// Keep the behavior of every data view consistent without changing its model.
void configureDataTable(QTableWidget* table) {
  table->verticalHeader()->setVisible(false);
  table->verticalHeader()->setDefaultSectionSize(48);
  table->setShowGrid(false);
  table->setAlternatingRowColors(true);
  table->setEditTriggers(QAbstractItemView::NoEditTriggers);
  table->setSelectionBehavior(QAbstractItemView::SelectRows);
  table->setFocusPolicy(Qt::StrongFocus);
  table->horizontalHeader()->setMinimumSectionSize(92);
  table->horizontalHeader()->setHighlightSections(false);
  table->setWordWrap(false);
}

void filterTableRows(QTableWidget* table, const QString& query) {
  const QString needle = query.trimmed();
  for (int row = 0; row < table->rowCount(); ++row) {
    bool match = needle.isEmpty();
    for (int col = 0; !match && col < table->columnCount(); ++col) {
      const auto* item = table->item(row, col);
      match = item && item->text().contains(needle, Qt::CaseInsensitive);
    }
    table->setRowHidden(row, !match);
  }
}

void filterBookingRows(QTableWidget* table, const QString& query, const QString& status,
                       const QDate& from, const QDate& to, const QString& lab = {}, const QString& department = {}) {
  const QString needle = query.trimmed();
  for (int row = 0; row < table->rowCount(); ++row) {
    bool textMatch = needle.isEmpty();
    for (int col = 0; !textMatch && col < table->columnCount(); ++col) {
      const auto* item = table->item(row, col);
      textMatch = item && item->text().contains(needle, Qt::CaseInsensitive);
    }
    const auto* dateItem = table->item(row, 3);
    const QDate date = dateItem ? dateItem->data(Qt::UserRole).toDate() : QDate();
    const bool dateMatch = date.isValid() && date >= from && date <= to;
    const auto* labItem = table->item(row, 2);
    const bool labMatch = lab.isEmpty() || (labItem && labItem->text().contains(lab, Qt::CaseInsensitive));
    const auto* requesterItem = table->item(row, 1);
    const bool departmentMatch = department.isEmpty() || (requesterItem &&
      requesterItem->data(Qt::UserRole + 1).toString().contains(department, Qt::CaseInsensitive));
    const auto* statusItem = table->item(row, 7);
    const bool statusMatch = status.isEmpty() || (statusItem && statusItem->text() == status);
    table->setRowHidden(row, !(textMatch && dateMatch && labMatch && departmentMatch && statusMatch));
  }
}

QWidget* calendarDateControl(QDateEdit* dateEdit) {
  auto* control = new QWidget;
  control->setFocusProxy(dateEdit);
  auto* row = new QHBoxLayout(control);
  row->setContentsMargins(0, 0, 0, 0);
  row->setSpacing(8);
  dateEdit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
  auto* openCalendar = new QPushButton;
  openCalendar->setObjectName("calendarPickerButton");
  openCalendar->setIcon(navigationIcon("availability", QColor("#ffffff")));
  openCalendar->setIconSize(QSize(20, 20));
  openCalendar->setFixedSize(46, 46);
  openCalendar->setCursor(Qt::PointingHandCursor);
  openCalendar->setToolTip("Choose a date from the calendar");
  openCalendar->setAccessibleName("Choose a date from the calendar");
  row->addWidget(dateEdit, 1);
  row->addWidget(openCalendar);

  QObject::connect(openCalendar, &QPushButton::clicked, dateEdit, [dateEdit] {
    QDialog dialog(dateEdit);
    dialog.setWindowTitle("Choose a booking date");
    dialog.setMinimumWidth(360);
    auto* layout = new QVBoxLayout(&dialog);
    layout->setContentsMargins(20, 20, 20, 20);
    layout->setSpacing(14);
    auto* heading = new QLabel("Choose a booking date");
    heading->setStyleSheet("font-size: 17px; font-weight: 700;");
    auto* hint = new QLabel("Laboratories are open Monday to Friday. Past dates are unavailable.");
    hint->setWordWrap(true);
    layout->addWidget(heading);
    layout->addWidget(hint);
    auto* calendar = new QCalendarWidget;
    calendar->setObjectName("bookingCalendar");
    calendar->setMinimumDate(dateEdit->minimumDate());
    calendar->setSelectedDate(dateEdit->date());
    calendar->setGridVisible(false);
    calendar->setFirstDayOfWeek(Qt::Monday);
    QTextCharFormat weekend;
    weekend.setForeground(QColor("#94a3b8"));
    calendar->setWeekdayTextFormat(Qt::Saturday, weekend);
    calendar->setWeekdayTextFormat(Qt::Sunday, weekend);
    layout->addWidget(calendar);
    auto* message = new QLabel;
    message->setWordWrap(true);
    layout->addWidget(message);
    auto* actions = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    auto* useDate = actions->button(QDialogButtonBox::Ok);
    useDate->setText("Use selected date");
    layout->addWidget(actions);
    auto validate = [calendar, message, useDate] {
      const bool weekday = !isWeekend(calendar->selectedDate());
      useDate->setEnabled(weekday);
      message->setText(weekday ? "This date is available for booking checks."
                               : "Weekends are closed. Please choose Monday to Friday.");
      message->setStyleSheet(weekday ? "color:#047857;" : "color:#b45309;");
    };
    QObject::connect(calendar, &QCalendarWidget::selectionChanged, &dialog, validate);
    QObject::connect(actions, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    QObject::connect(actions, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
    validate();
    if (dialog.exec() == QDialog::Accepted) dateEdit->setDate(calendar->selectedDate());
  });
  return control;
}

void styleFormDialog(QDialog& dialog, QFormLayout& form) {
  dialog.setMinimumWidth(540);
  form.setContentsMargins(28, 26, 28, 26);
  form.setHorizontalSpacing(20);
  form.setVerticalSpacing(14);
  form.setFieldGrowthPolicy(QFormLayout::ExpandingFieldsGrow);
  dialog.setStyleSheet(
    "QDialog { background: #ffffff; color: #0f172a; font-family: 'Segoe UI'; }"
    "QDialog QLabel { color: #334155; font-size: 13px; font-weight: 600; }"
    "QDialog QLineEdit, QDialog QComboBox, QDialog QSpinBox {"
    "  background: #f8fafc; color: #0f172a; border: 1px solid #cbd5e1; border-radius: 8px; padding: 8px 12px; min-height: 22px; font-size: 13px;"
    "}"
    "QDialog QLineEdit:focus, QDialog QComboBox:focus, QDialog QSpinBox:focus {"
    "  background: #ffffff; border: 1px solid #6366f1;"
    "}"
    "QDialog QPushButton {"
    "  background: #4f46e5; color: white; border: 0; border-radius: 8px; padding: 10px 18px; font-weight: 600; font-size: 13px; min-height: 20px;"
    "}"
    "QDialog QPushButton:hover { background: #4338ca; }"
  );
}

} // namespace

// ============================================================================
// LoginWindow
// ============================================================================

LoginWindow::LoginWindow(DatabaseManager& db, QWidget* parent)
  : QWidget(parent), m_db(db), m_loginBackground(":/brand/login-background.png") {
  setWindowTitle("LabBooker - University Laboratory Booking System");
  setMinimumSize(850, 750);
  resize(1100, 850);
  m_backgroundLabel = new QLabel(this);
  m_backgroundLabel->setObjectName("loginBackground");
  m_backgroundLabel->setAlignment(Qt::AlignCenter);
  m_backgroundLabel->setGeometry(rect());
  m_backgroundLabel->setPixmap(m_loginBackground.scaled(size(), Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation));
  m_backgroundLabel->lower();

  auto* outer = new QVBoxLayout(this);
  outer->setContentsMargins(24, 24, 24, 24);
  outer->setAlignment(Qt::AlignCenter);

  auto* card = new QFrame;
  card->setObjectName("loginCard");
  card->setFixedWidth(460);

  auto* shadow = new QGraphicsDropShadowEffect(card);
  shadow->setBlurRadius(30);
  shadow->setOffset(0, 8);
  shadow->setColor(QColor(15, 23, 42, 22));
  card->setGraphicsEffect(shadow);

  auto* content = new QVBoxLayout(card);
  content->setContentsMargins(36, 32, 36, 32);
  content->setSpacing(0);

  // 1. Centered Logo
  auto* logoLabel = new QLabel;
  logoLabel->setObjectName("loginLogo");
  logoLabel->setAlignment(Qt::AlignCenter);
  QPixmap rawLogo(":/brand/labbooker.png");
  if (!rawLogo.isNull()) {
    logoLabel->setPixmap(rawLogo.scaledToWidth(210, Qt::SmoothTransformation));
  }
  content->addWidget(logoLabel);
  content->addSpacing(14);

  // 2. Heading
  auto* heading = new QLabel("Sign in to your account");
  heading->setObjectName("loginHeading");
  heading->setAlignment(Qt::AlignCenter);
  content->addWidget(heading);
  content->addSpacing(26);

  // 3. Field 1: University Email / Student ID
  auto* idHeaderLayout = new QHBoxLayout;
  auto* idLabel = new QLabel("Student / Staff ID (E-ID)");
  idLabel->setObjectName("fieldCaption");
  auto* idHelper = new QLabel("Use your campus ID");
  idHelper->setObjectName("fieldHelper");
  idHeaderLayout->addWidget(idLabel);
  idHeaderLayout->addStretch();
  idHeaderLayout->addWidget(idHelper);
  content->addLayout(idHeaderLayout);
  content->addSpacing(8);

  auto* idShell = new QFrame;
  idShell->setObjectName("inputShell");
  idShell->setFixedHeight(50);
  auto* idLayout = new QHBoxLayout(idShell);
  idLayout->setContentsMargins(16, 0, 10, 0);
  idLayout->setSpacing(12);

  auto* idBadge = new QLabel("ID");
  idBadge->setObjectName("inputBadge");
  idLayout->addWidget(idBadge);

  m_eid = new QLineEdit;
  m_eid->setObjectName("loginInput");
  m_eid->setPlaceholderText("249109");
  m_eid->setClearButtonEnabled(true);
  idLayout->addWidget(m_eid, 1);
  content->addWidget(idShell);
  content->addSpacing(18);

  // 4. Field 2: Password
  auto* pwLabel = new QLabel("Password");
  pwLabel->setObjectName("fieldCaption");
  content->addWidget(pwLabel);
  content->addSpacing(8);

  auto* pwShell = new QFrame;
  pwShell->setObjectName("inputShell");
  pwShell->setFixedHeight(50);
  auto* pwLayout = new QHBoxLayout(pwShell);
  pwLayout->setContentsMargins(16, 0, 10, 0);
  pwLayout->setSpacing(12);

  auto* pwBadge = new QLabel("PW");
  pwBadge->setObjectName("inputBadge");
  pwLayout->addWidget(pwBadge);

  m_password = new QLineEdit;
  m_password->setObjectName("loginInput");
  m_password->setEchoMode(QLineEdit::Password);
  m_password->setPlaceholderText("Enter your password");
  pwLayout->addWidget(m_password, 1);

  auto* reveal = new QPushButton("Show");
  reveal->setObjectName("revealButton");
  reveal->setCheckable(true);
  reveal->setCursor(Qt::PointingHandCursor);
  pwLayout->addWidget(reveal);
  content->addWidget(pwShell);
  content->addSpacing(18);

  // 5. Options Row: Remember me + Forgot password?
  auto* options = new QHBoxLayout;
  m_remember = new QCheckBox("Remember me");
  m_remember->setObjectName("rememberCheck");
  m_remember->setCursor(Qt::PointingHandCursor);
  auto* forgot = new QPushButton("Forgot password?");
  forgot->setObjectName("forgotLink");
  forgot->setCursor(Qt::PointingHandCursor);
  options->addWidget(m_remember);
  options->addStretch();
  options->addWidget(forgot);
  content->addLayout(options);
  content->addSpacing(22);

  // 6. Sign in Button
  auto* signIn = new QPushButton("Sign in");
  signIn->setObjectName("loginSubmit");
  signIn->setCursor(Qt::PointingHandCursor);
  signIn->setFixedHeight(50);
  content->addWidget(signIn);
  content->addSpacing(10);

  // Status message
  m_status = new QLabel;
  m_status->setObjectName("loginStatus");
  m_status->setWordWrap(true);
  m_status->setAlignment(Qt::AlignCenter);
  content->addWidget(m_status);
  content->addSpacing(14);

  // 7. Divider
  auto* divider = new QFrame;
  divider->setFrameShape(QFrame::HLine);
  divider->setObjectName("loginDivider");
  content->addWidget(divider);
  content->addSpacing(18);

  // 8. Footer prompt and links
  auto* footerPrompt = new QLabel("Need access or new student registration?");
  footerPrompt->setObjectName("footerPrompt");
  footerPrompt->setAlignment(Qt::AlignCenter);
  content->addWidget(footerPrompt);
  content->addSpacing(8);

  auto* contactAdmin = new QPushButton("Contact your lab administrator");
  contactAdmin->setObjectName("footerLink");
  contactAdmin->setCursor(Qt::PointingHandCursor);
  content->addWidget(contactAdmin);
  content->addSpacing(6);

  auto* registerAccount = new QPushButton("Create a student account");
  registerAccount->setObjectName("footerLink");
  registerAccount->setCursor(Qt::PointingHandCursor);
  content->addWidget(registerAccount);

  outer->addWidget(card);

  setStyleSheet(
    "LoginWindow { font-family: 'Segoe UI', sans-serif; }"
    "QFrame#loginCard { background: #ffffff; border: 1.5px solid #edf2f7; border-radius: 20px; }"
    "QLabel#loginHeading { font-size: 25px; font-weight: 700; color: #0f172a; letter-spacing: -0.2px; }"
    "QLabel#fieldCaption { font-size: 13.5px; font-weight: 600; color: #1e293b; }"
    "QLabel#fieldHelper { font-size: 12.5px; color: #94a3b8; font-weight: 400; }"
    "QFrame#inputShell { background: #f8fafc; border: 1.5px solid #e2e8f0; border-radius: 12px; }"
    "QFrame#inputShell:focus-within { background: #ffffff; border: 1.5px solid #6366f1; }"
    "QLabel#inputBadge { color: #94a3b8; font-weight: 700; font-size: 13px; background: transparent; }"
    "QLineEdit#loginInput { background: transparent; border: 0; font-size: 15px; color: #0f172a; selection-background-color: #6366f1; }"
    "QPushButton#revealButton { background: transparent; color: #64748b; border: 0; padding: 4px 6px; font-weight: 600; font-size: 13px; }"
    "QPushButton#revealButton:hover { color: #6366f1; }"
    "QCheckBox#rememberCheck { font-size: 13.5px; color: #475569; font-weight: 500; }"
    "QCheckBox#rememberCheck::indicator { width: 18px; height: 18px; border: 1.5px solid #cbd5e1; border-radius: 5px; background: #ffffff; }"
    "QCheckBox#rememberCheck::indicator:checked { background: #6366f1; border-color: #6366f1; }"
    "QPushButton#forgotLink { background: transparent; color: #6366f1; border: 0; font-weight: 600; font-size: 13.5px; }"
    "QPushButton#forgotLink:hover { color: #4f46e5; text-decoration: underline; }"
    "QPushButton#loginSubmit { background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #6366f1, stop:1 #7c3aed); color: #ffffff; border: 0; border-radius: 12px; font-size: 16px; font-weight: 700; }"
    "QPushButton#loginSubmit:hover { background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #4f46e5, stop:1 #6d28d9); }"
    "QLabel#loginStatus { color: #dc2626; font-size: 13px; font-weight: 500; min-height: 20px; }"
    "QFrame#loginDivider { color: #edf2f7; border: 0; border-top: 1px solid #e2e8f0; }"
    "QLabel#footerPrompt { color: #64748b; font-size: 13px; font-weight: 400; }"
    "QPushButton#footerLink { background: transparent; color: #6366f1; border: 0; font-weight: 600; font-size: 13.5px; }"
    "QPushButton#footerLink:hover { color: #4f46e5; text-decoration: underline; }"
    "QDialog, QMessageBox, QInputDialog { background-color: #ffffff; color: #0f172a; }"
    "QMessageBox QLabel, QInputDialog QLabel, QDialog QLabel { background-color: transparent; color: #0f172a; font-size: 13px; font-weight: 500; }"
    "QMessageBox QLabel#qt_msgbox_label { color: #0f172a; font-size: 14px; font-weight: 600; }"
    "QMessageBox QLabel#qt_msgbox_informativelabel { color: #475569; font-size: 13px; }"
    "QMessageBox QPushButton, QInputDialog QPushButton { background: #6366f1; color: white; border: 0; border-radius: 8px; padding: 8px 20px; font-weight: 600; font-size: 13px; min-width: 70px; min-height: 24px; }"
    "QMessageBox QPushButton:hover, QInputDialog QPushButton:hover { background: #4f46e5; }"
  );

  QSettings settings;
  m_remember->setChecked(settings.value("login/rememberIdentifier", false).toBool());
  if (m_remember->isChecked()) m_eid->setText(settings.value("login/identifier").toString());

  connect(reveal, &QPushButton::toggled, this, [this, reveal](bool checked) {
    m_password->setEchoMode(checked ? QLineEdit::Normal : QLineEdit::Password);
    reveal->setText(checked ? "Hide" : "Show");
  });
  connect(signIn, &QPushButton::clicked, this, &LoginWindow::login);
  connect(m_password, &QLineEdit::returnPressed, this, &LoginWindow::login);
  connect(m_eid, &QLineEdit::returnPressed, this, [this] { m_password->setFocus(); });
  connect(forgot, &QPushButton::clicked, this, [this] {
    QMessageBox::information(this, "Password Help",
      "For account security, passwords cannot be retrieved directly.\nPlease contact your laboratory administrator to reset your credentials.");
  });
  connect(contactAdmin, &QPushButton::clicked, this, [this] {
    const QString contact = QSettings().value("university/adminContact").toString().trimmed();
    QMessageBox::information(this, "Laboratory Administrator Contact",
      contact.isEmpty() ? "Ask your university department for the laboratory administrator's contact details."
                        : contact);
  });
  connect(registerAccount, &QPushButton::clicked, this, &LoginWindow::requestAccount);
}
void LoginWindow::resizeEvent(QResizeEvent* event) {
  QWidget::resizeEvent(event);
  if (!m_backgroundLabel || m_loginBackground.isNull()) return;
  m_backgroundLabel->setGeometry(rect());
  m_backgroundLabel->setPixmap(m_loginBackground.scaled(size(), Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation));
  m_backgroundLabel->lower();
}

void LoginWindow::reset() {
  m_password->clear();
  m_status->clear();
  m_eid->setFocus();
}

void LoginWindow::login() {
  const QString entered = m_eid->text().trimmed();
  QString eid = entered;
  if (entered.contains('@')) {
    for (const auto& u : m_db.users(entered, true)) {
      if (u.active && u.email.compare(entered, Qt::CaseInsensitive) == 0) {
        eid = u.eid;
        break;
      }
    }
  }

  QString error;
  auto user = AuthService(m_db).authenticate(eid, m_password->text(), &error);
  if (!user) {
    m_status->setText(error);
    return;
  }

  QSettings settings;
  settings.setValue("login/rememberIdentifier", m_remember->isChecked());
  if (m_remember->isChecked()) settings.setValue("login/identifier", entered);
  else settings.remove("login/identifier");

  if (user->mustChangePassword()) {
    QDialog dialog(this);
    dialog.setWindowTitle("Set Permanent Password");
    auto* form = new QFormLayout(&dialog);
    styleFormDialog(dialog, *form);
    auto* next = new QLineEdit; next->setEchoMode(QLineEdit::Password);
    auto* confirm = new QLineEdit; confirm->setEchoMode(QLineEdit::Password);
    auto* save = createButton("Update Password");
    form->addRow("New Password (8+ chars)", next);
    form->addRow("Confirm Password", confirm);
    form->addRow(save);

    bool changed = false;
    const QString current = m_password->text();
    connect(save, &QPushButton::clicked, &dialog, [&, user] {
      if (next->text().size() < 8) {
        QMessageBox::warning(&dialog, "Password Rule", "Password must be at least 8 characters long.");
        return;
      }
      if (next->text() != confirm->text()) {
        QMessageBox::warning(&dialog, "Password Match", "The passwords do not match.");
        return;
      }
      const auto result = AuthService(m_db).changePassword(user->id(), current, next->text());
      if (!result.ok) {
        QMessageBox::warning(&dialog, "Change Failed", result.error);
        return;
      }
      changed = true;
      dialog.accept();
    });
    if (dialog.exec() != QDialog::Accepted || !changed) return;
  }

  emit authenticated(std::move(user));
}

void LoginWindow::requestAccount() {
  QDialog dialog(this);
  dialog.setWindowTitle("Request Student Account");
  auto* form = new QFormLayout(&dialog);
  styleFormDialog(dialog, *form);

  auto* eid = new QLineEdit; eid->setPlaceholderText("e.g. 249110");
  auto* name = new QLineEdit; name->setPlaceholderText("Full Name");
  auto* email = new QLineEdit; email->setPlaceholderText("student@university.edu");
  auto* faculty = new QLineEdit; faculty->setPlaceholderText("Faculty of Science / Engineering");
  auto* department = new QLineEdit; department->setPlaceholderText("Department");
  auto* contact = new QLineEdit; contact->setPlaceholderText("Phone / Contact");
  auto* pw = new QLineEdit; pw->setEchoMode(QLineEdit::Password); pw->setPlaceholderText("8+ characters");
  auto* confirm = new QLineEdit; confirm->setEchoMode(QLineEdit::Password);
  auto* submit = createButton("Submit Account Request");

  form->addRow("Student E-ID", eid);
  form->addRow("Full Name", name);
  form->addRow("University Email", email);
  form->addRow("Faculty", faculty);
  form->addRow("Department", department);
  form->addRow("Contact", contact);
  form->addRow("Password", pw);
  form->addRow("Confirm Password", confirm);
  form->addRow(submit);

  connect(submit, &QPushButton::clicked, &dialog, [this, &dialog, eid, name, email, faculty, department, contact, pw, confirm] {
    if (pw->text().size() < 8) {
      QMessageBox::warning(&dialog, "Password Rule", "Password must be at least 8 characters.");
      return;
    }
    if (pw->text() != confirm->text()) {
      QMessageBox::warning(&dialog, "Validation", "Passwords do not match.");
      return;
    }
    UserRecord r;
    r.eid = eid->text().trimmed();
    r.name = name->text().trimmed();
    r.email = email->text().trimmed();
    r.faculty = faculty->text().trimmed();
    r.department = department->text().trimmed();
    r.contact = contact->text().trimmed();
    r.role = UserRole::Student;

    const auto result = AuthService(m_db).registerAccount(r, pw->text());
    if (!result.ok) {
      QMessageBox::warning(&dialog, "Request Failed", result.error);
      return;
    }
    m_status->setText("Account request submitted. Waiting for administrator approval.");
    QMessageBox::information(&dialog, "Submitted", "Your account has been submitted for administrator review.");
    dialog.accept();
  });

  dialog.exec();
}

// ============================================================================
// MainWindow
// ============================================================================

MainWindow::MainWindow(DatabaseManager& db, std::shared_ptr<User> u, QWidget* p)
  : QMainWindow(p), m_db(db), m_user(std::move(u)) {
  setWindowTitle("LabBooker - University Laboratory Booking System");
  resize(1280, 840);

  auto* root = new QWidget;
  auto* layout = new QHBoxLayout(root);
  layout->setContentsMargins(0, 0, 0, 0);
  layout->setSpacing(0);

  // --- Sidebar ---
  auto* sidebar = new QFrame;
  sidebar->setObjectName("sidebar");
  sidebar->setFixedWidth(280);
  auto* side = new QVBoxLayout(sidebar);
  side->setContentsMargins(0, 0, 0, 16);
  side->setSpacing(2);

  // Brand Header
  auto* brandHeader = new QFrame;
  brandHeader->setObjectName("brandHeader");
  auto* brandLayout = new QHBoxLayout(brandHeader);
  brandLayout->setContentsMargins(20, 20, 20, 20);
  brandLayout->setSpacing(14);
  auto* brandIcon = new QLabel;
  brandIcon->setObjectName("brandIcon");
  brandIcon->setAlignment(Qt::AlignCenter);
  brandIcon->setFixedSize(44, 44);
  brandIcon->setPixmap(flaskMark());
  brandLayout->addWidget(brandIcon);

  auto* brandText = new QVBoxLayout;
  brandText->setContentsMargins(0, 0, 0, 0);
  brandText->setSpacing(1);
  auto* brandTitle = new QLabel("LabBooker");
  brandTitle->setObjectName("brandTitle");
  auto* brandSubtitle = new QLabel("University Portal");
  brandSubtitle->setObjectName("brandSubtitle");
  brandText->addWidget(brandTitle);
  brandText->addWidget(brandSubtitle);
  brandLayout->addLayout(brandText);
  brandLayout->addStretch();
  auto* collapseNav = new QPushButton("‹");
  collapseNav->setObjectName("collapseNav");
  collapseNav->setFixedSize(32, 32);
  collapseNav->setToolTip("Collapse navigation");
  collapseNav->setAccessibleName("Collapse navigation");
  brandLayout->addWidget(collapseNav);
  side->addWidget(brandHeader);

  auto addSection = [&](const QString& label) {
    auto* sec = new QLabel(label);
    sec->setObjectName("navSection");
    side->addWidget(sec);
  };

  m_pages = new QStackedWidget;
  bool firstNav = true;

  auto addNav = [&](const QString& title, const QString& iconKey, QWidget* page) {
    auto* b = new QPushButton(title);
    b->setObjectName(firstNav ? "navButtonActive" : "navButton");
    firstNav = false;
    b->setProperty("navPage", title);
    b->setProperty("navIcon", iconKey);
    b->setProperty("fullLabel", title);
    b->setToolTip(title);
    b->setIcon(navigationIcon(iconKey, b->objectName() == "navButtonActive" ? QColor(Qt::white) : QColor("#94a3b8")));
    b->setIconSize(QSize(20, 20));
    b->setMinimumHeight(44);
    b->setCursor(Qt::PointingHandCursor);
    side->addWidget(b);
    m_navButtons.append(b);
    addPage(title, page);

    connect(b, &QPushButton::clicked, this, [this, page, title, b] {
      m_pages->setCurrentWidget(page);
      m_title->setText(title);
      refreshLaboratoryCombos();
      for (auto* btn : m_navButtons) {
        const bool active = (btn == b);
        btn->setObjectName(active ? "navButtonActive" : "navButton");
        btn->setIcon(navigationIcon(btn->property("navIcon").toString(), active ? QColor(Qt::white) : QColor("#94a3b8")));
        btn->style()->unpolish(btn);
        btn->style()->polish(btn);
        btn->update();
      }
      refreshCurrentPage();
    });
  };

  addSection("OVERVIEW");
  addNav("Dashboard", "dashboard", dashboard());
  addNav("Availability", "availability", availability());

  if (m_user->role() == UserRole::Administrator) {
    addSection("MANAGEMENT");
    addNav("All Bookings", "allbookings", adminBookings());
    addNav("Approvals", "approvals", approvals());
    addNav("Laboratories", "laboratories", laboratoriesPage());
    addNav("Equipment", "equipment", equipmentPage());
    addNav("Users", "users", usersPage());
    addNav("Reports", "reports", reportsPage());
  }

  addSection("RESERVATIONS");
  auto* newBookingPage = bookings();
  addNav("My Bookings", "bookings", m_myBookingsPage);
  addNav("New Booking", "bookings", newBookingPage);

  addSection("ACCOUNT");
  addNav("Profile", "profile", profilePage());
  addNav("Updates", "profile", updatesPage());
  addNav("Settings", "settings", settingsPage());

  side->addStretch();

  auto* logout = new QPushButton("Sign Out");
  logout->setObjectName("navButton");
  logout->setIcon(navigationIcon("logout", QColor("#94a3b8")));
  logout->setIconSize(QSize(20, 20));
  logout->setMinimumHeight(44);
  logout->setCursor(Qt::PointingHandCursor);
  logout->setProperty("fullLabel", "Sign Out");
  logout->setToolTip("Sign Out");
  side->addWidget(logout);
  connect(logout, &QPushButton::clicked, this, [this] { emit logoutRequested(); });

  connect(collapseNav, &QPushButton::clicked, this,
          [sidebar, brandIcon, brandTitle, brandSubtitle, brandLayout, collapseNav] {
    const bool collapsed = sidebar->width() > 100;
    sidebar->setFixedWidth(collapsed ? 82 : 280);
    brandLayout->setContentsMargins(collapsed ? 12 : 20, 20, collapsed ? 12 : 20, 20);
    brandIcon->setVisible(!collapsed);
    brandTitle->setVisible(!collapsed);
    brandSubtitle->setVisible(!collapsed);
    for (auto* section : sidebar->findChildren<QLabel*>("navSection"))
      section->setVisible(!collapsed);
    for (auto* button : sidebar->findChildren<QPushButton*>()) {
      if (button == collapseNav || !button->property("fullLabel").isValid()) continue;
      button->setText(collapsed ? QString() : button->property("fullLabel").toString());
    }
    collapseNav->setText(collapsed ? "›" : "‹");
    collapseNav->setToolTip(collapsed ? "Expand navigation" : "Collapse navigation");
    collapseNav->setAccessibleName(collapseNav->toolTip());
  });

  layout->addWidget(sidebar);

  // --- Main Content Area ---
  auto* mainCol = new QWidget;
  auto* content = new QVBoxLayout(mainCol);
  content->setContentsMargins(0, 0, 0, 0);
  content->setSpacing(0);

  // Topbar
  auto* topbar = new QFrame;
  topbar->setObjectName("topbar");
  auto* top = new QHBoxLayout(topbar);
  top->setContentsMargins(28, 16, 28, 16);

  auto* titleCol = new QVBoxLayout;
  titleCol->setSpacing(2);
  m_title = new QLabel("Dashboard");
  m_title->setObjectName("pageTitle");
  titleCol->addWidget(m_title);
  top->addLayout(titleCol);
  top->addStretch();

  auto* userCard = new QWidget;
  userCard->setObjectName("userProfile");
  auto* userLayout = new QHBoxLayout(userCard);
  userLayout->setContentsMargins(0, 0, 0, 0);
  userLayout->setSpacing(12);

  auto* userAvatar = new QLabel(userInitials(m_user->name()));
  userAvatar->setObjectName("topProfileAvatar");
  userAvatar->setAlignment(Qt::AlignCenter);
  userAvatar->setFixedSize(38, 38);
  userLayout->addWidget(userAvatar);

  auto* userDetails = new QVBoxLayout;
  userDetails->setContentsMargins(0, 0, 0, 0);
  userDetails->setSpacing(1);
  auto* userName = new QLabel(m_user->name());
  userName->setObjectName("topProfileName");
  auto* userRole = new QLabel(m_user->roleName());
  userRole->setObjectName("topProfileRole");
  userDetails->addWidget(userName);
  userDetails->addWidget(userRole);
  userLayout->addLayout(userDetails);
  top->addWidget(userCard);

  // Load avatar image if available
  const QString savedPhoto = QSettings().value(QString("profiles/%1/image").arg(m_user->id())).toString();
  QPixmap topPhoto(savedPhoto);
  if (!topPhoto.isNull()) {
    topPhoto = topPhoto.scaled(38, 38, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);
    QPixmap roundPhoto(38, 38);
    roundPhoto.fill(Qt::transparent);
    QPainter pAvatar(&roundPhoto);
    pAvatar.setRenderHint(QPainter::Antialiasing);
    QPainterPath clip;
    clip.addEllipse(0, 0, 38, 38);
    pAvatar.setClipPath(clip);
    pAvatar.drawPixmap(0, 0, topPhoto, (topPhoto.width() - 38) / 2, (topPhoto.height() - 38) / 2, 38, 38);
    userAvatar->setText(QString());
    userAvatar->setPixmap(roundPhoto);
  }

  content->addWidget(topbar);
  content->addWidget(m_pages, 1);
  layout->addWidget(mainCol, 1);

  setCentralWidget(root);
  refreshDashboard();
  refreshLaboratoryCombos();
  updateNavBadges();

  QSettings pref;
  applyTheme(pref.value("appearance/theme", "Light").toString());
}

void MainWindow::addPage(const QString&, QWidget* w) {
  m_pages->addWidget(w);
}

void MainWindow::navigateToPage(const QString& title) {
  for (int i = 0; i < m_navButtons.size(); ++i) {
    auto* btn = m_navButtons[i];
    if (btn->property("navPage").toString().compare(title, Qt::CaseInsensitive) == 0) {
      btn->click();
      return;
    }
  }
}

void MainWindow::prefillBookingForm(qint64 labId, const QDate& date, const QTime& start, const QTime& end) {
  if (m_bookingLabCombo) {
    const int idx = m_bookingLabCombo->findData(labId);
    if (idx >= 0) m_bookingLabCombo->setCurrentIndex(idx);
  }
  if (m_bookingDateEdit) m_bookingDateEdit->setDate(date);
  if (m_bookingStartEdit) m_bookingStartEdit->setTime(start);
  if (m_bookingEndEdit) m_bookingEndEdit->setTime(end);
  std::optional<FixedBookingSlot> slotOpt;
  for (const auto& slot : m_db.bookingSlots()) if (slot.start == start && slot.end == end) slotOpt = slot;
  if (slotOpt) {
    m_selectedSlotId = slotOpt->id;
  }
  if (m_refreshBookingSlotsFn) {
    m_refreshBookingSlotsFn();
  }
}

void MainWindow::refreshLaboratoryCombos() {
  const auto labs = m_db.laboratories(false);
  if (m_availabilitySlotCombo) {
    const int selected = m_availabilitySlotCombo->currentData().toInt();
    m_availabilitySlotCombo->blockSignals(true);
    m_availabilitySlotCombo->clear();
    for (const auto& slot : m_db.bookingSlots()) m_availabilitySlotCombo->addItem(slot.fullTitle, slot.id);
    const int index = m_availabilitySlotCombo->findData(selected);
    m_availabilitySlotCombo->setCurrentIndex(index >= 0 ? index : 0);
    m_availabilitySlotCombo->blockSignals(false);
  }

  if (m_availabilityLabCombo) {
    const qint64 prevLabId = m_availabilityLabCombo->currentData().toLongLong();
    m_availabilityLabCombo->blockSignals(true);
    m_availabilityLabCombo->clear();
    if (labs.isEmpty()) {
      m_availabilityLabCombo->addItem("No active laboratories", -1);
      m_availabilityLabCombo->setEnabled(false);
    } else {
      m_availabilityLabCombo->setEnabled(true);
      int selectIdx = 0;
      for (int i = 0; i < labs.size(); ++i) {
        const auto& l = labs[i];
        m_availabilityLabCombo->addItem(QString("%1 (%2) — Capacity %3").arg(l.name, l.number).arg(l.capacity), l.id);
        if (l.id == prevLabId) {
          selectIdx = i;
        }
      }
      m_availabilityLabCombo->setCurrentIndex(selectIdx);
    }
    m_availabilityLabCombo->blockSignals(false);
    emit m_availabilityLabCombo->currentIndexChanged(m_availabilityLabCombo->currentIndex());
  }

  if (m_bookingLabCombo) {
    const qint64 prevLabId = m_bookingLabCombo->currentData().toLongLong();
    m_bookingLabCombo->blockSignals(true);
    m_bookingLabCombo->clear();
    if (labs.isEmpty()) {
      m_bookingLabCombo->addItem("No active laboratories", -1);
      m_bookingLabCombo->setEnabled(false);
    } else {
      m_bookingLabCombo->setEnabled(true);
      int selectIdx = 0;
      for (int i = 0; i < labs.size(); ++i) {
        const auto& l = labs[i];
        m_bookingLabCombo->addItem(QString("%1 [%2] — Cap: %3").arg(l.name, l.number).arg(l.capacity), l.id);
        if (l.id == prevLabId) {
          selectIdx = i;
        }
      }
      m_bookingLabCombo->setCurrentIndex(selectIdx);
    }
    m_bookingLabCombo->blockSignals(false);
    emit m_bookingLabCombo->currentIndexChanged(m_bookingLabCombo->currentIndex());
  }

  if (m_refreshEquipmentFn) {
    m_refreshEquipmentFn();
  }
  if (m_refreshBookingSlotsFn) {
    m_refreshBookingSlotsFn();
  }
}

void MainWindow::updateNavBadges() {
  const int myBookingCount = m_db.bookingsForUser(m_user->id()).size();
  const bool admin = m_user->role() == UserRole::Administrator;
  const int pendingApprovals = admin ? m_db.allBookings(true).size() : 0;
  const int pendingUsers = admin ? m_db.count("users", "approved = 0") : 0;
  const QString lastSeen = QSettings().value(QString("updates/seen/%1").arg(m_user->id())).toString();
  int newUpdates = 0;
  for (const auto& notification : m_db.notificationsForUser(m_user->id()))
    if (lastSeen.isEmpty() || notification.timestamp > lastSeen) ++newUpdates;
  const auto* sidebar = findChild<QFrame*>("sidebar");
  const bool collapsed = sidebar && sidebar->width() < 100;

  for (auto* b : m_navButtons) {
    const QString page = b->property("navPage").toString();
    QString label = page;
    if (page == "Approvals") {
      if (pendingApprovals > 0) label = QString("Approvals (%1)").arg(pendingApprovals);
    } else if (page == "Users") {
      if (pendingUsers > 0) label = QString("Users (%1)").arg(pendingUsers);
    } else if (page == "My Bookings") {
      label = QString("My Bookings (%1)").arg(myBookingCount);
    } else if (page == "Updates" && newUpdates > 0) {
      label = QString("Updates (%1)").arg(newUpdates);
    }
    b->setProperty("fullLabel", label);
    b->setToolTip(label);
    b->setText(collapsed ? QString() : label);
  }
}

void MainWindow::applyTheme(const QString& theme) {
  const bool dark = theme.compare("Dark", Qt::CaseInsensitive) == 0;

  if (dark) {
    setStyleSheet(
      "QMainWindow { background: #0b0f19; }"
      "QWidget { font-family: 'Segoe UI'; font-size: 13px; color: #f1f5f9; }"
      "QFrame#sidebar { background: #080d18; border-right: 1px solid #1e293b; }"
      "QFrame#brandHeader { background: transparent; border-bottom: 1px solid #1e293b; }"
      "QLabel#brandIcon { background: qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 #4f46e5, stop:1 #7c3aed); border-radius: 12px; }"
      "QLabel#brandTitle { color: #f8fafc; font-size: 18px; font-weight: 700; }"
      "QLabel#brandSubtitle { color: #64748b; font-size: 12px; }"
      "QLabel#navSection { color: #475569; font-size: 11px; font-weight: 700; letter-spacing: 1px; padding: 16px 20px 4px; }"
      "QPushButton#navButton { background: transparent; color: #94a3b8; text-align: left; border: 0; border-radius: 10px; padding: 10px 16px; margin: 2px 12px; font-size: 14px; font-weight: 500; }"
      "QPushButton#navButton:hover { background: #1e293b; color: #f8fafc; }"
      "QPushButton#navButtonActive { background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #4f46e5, stop:1 #6366f1); color: white; text-align: left; border: 0; border-radius: 10px; padding: 10px 16px; margin: 2px 12px; font-size: 14px; font-weight: 700; }"
      "QPushButton#collapseNav { background: #1e293b; color: #f8fafc; border: 0; border-radius: 8px; padding: 0; font-size: 19px; }"
      "QPushButton#collapseNav:hover { background: #334155; }"
      "QFrame#topbar { background: #111827; border-bottom: 1px solid #1e293b; }"
      "QLabel#pageTitle { font-size: 22px; font-weight: 700; color: #f8fafc; }"
      "QLabel#topProfileAvatar { background: qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 #4f46e5, stop:1 #06b6d4); color: white; border-radius: 19px; font-size: 13px; font-weight: 700; }"
      "QLabel#topProfileName { color: #f8fafc; font-size: 13px; font-weight: 600; }"
      "QLabel#topProfileRole { color: #94a3b8; font-size: 11px; }"
      "QStackedWidget { background: #0b0f19; }"
      "QFrame#card, QFrame#formCard, QFrame#settingsCard, QFrame#statCard { background: #111827; border: 1px solid #1e293b; border-radius: 14px; }"
      "QPushButton { background: #4f46e5; color: white; border: 0; border-radius: 8px; padding: 9px 16px; font-weight: 600; font-size: 13px; }"
      "QPushButton:hover { background: #4338ca; }"
      "QPushButton:disabled { background: #1e293b; color: #64748b; }"
      "QPushButton[btnStyle='secondary'] { background: #1e293b; color: #e2e8f0; border: 1px solid #334155; }"
      "QPushButton[btnStyle='secondary']:hover { background: #334155; }"
      "QPushButton[btnStyle='success'] { background: #10b981; color: white; }"
      "QPushButton[btnStyle='success']:hover { background: #059669; }"
      "QPushButton[btnStyle='danger'] { background: #ef4444; color: white; }"
      "QPushButton[btnStyle='danger']:hover { background: #dc2626; }"
      "QPushButton[btnStyle='success']:disabled, QPushButton[btnStyle='danger']:disabled, QPushButton[btnStyle='secondary']:disabled { background: #1e293b; color: #64748b; border: 1px solid #334155; }"
      "QLineEdit, QDateEdit, QTimeEdit, QComboBox, QSpinBox {"
      "  background: #151d2d; color: #f8fafc; border: 1px solid #334155; border-radius: 8px; padding: 8px 12px; min-height: 22px; selection-background-color: #6366f1;"
      "}"
      "QLineEdit:focus, QDateEdit:focus, QTimeEdit:focus, QComboBox:focus, QSpinBox:focus { border: 1px solid #818cf8; }"
      "QFrame#availabilityField { background: #151d2d; border: 1px solid #334155; border-radius: 12px; }"
      "QLabel#availabilityFieldLabel { color: #e2e8f0; font-size: 13px; font-weight: 700; }"
      "QLabel#availabilityFieldHint { color: #94a3b8; font-size: 11px; }"
      "QFrame#availabilityField QComboBox, QFrame#availabilityField QDateEdit { background: #111827; color: #f8fafc; border: 1px solid #475569; border-radius: 8px; min-height: 30px; padding: 7px 38px 7px 12px; font-size: 13px; }"
      "QFrame#availabilityField QComboBox:focus, QFrame#availabilityField QDateEdit:focus { border: 2px solid #818cf8; }"
      "QFrame#availabilityField QComboBox::drop-down, QFrame#availabilityField QDateEdit::drop-down { width: 30px; border: 0; }"
      "QPushButton#calendarPickerButton { background: #4f46e5; border: 0; border-radius: 9px; padding: 0; }"
      "QPushButton#calendarPickerButton:hover { background: #6366f1; }"
      "QTableWidget { background: #111827; color: #f1f5f9; border: 1px solid #1e293b; border-radius: 10px; gridline-color: #1e293b; alternate-background-color: #151d2d; selection-background-color: #312e81; selection-color: white; }"
      "QHeaderView::section { background: #151d2d; color: #94a3b8; border: 0; border-bottom: 1px solid #1e293b; padding: 10px 12px; font-weight: 600; font-size: 12px; }"
      "QLabel#reportView { background: #111827; border: 1px solid #1e293b; border-radius: 12px; padding: 24px; color: #f1f5f9; }"
      "QScrollBar:vertical { background: #0b0f19; width: 10px; border-radius: 5px; }"
      "QScrollBar::handle:vertical { background: #334155; border-radius: 5px; min-height: 20px; }"
      "QDialog, QMessageBox, QInputDialog { background-color: #111827; color: #f1f5f9; }"
      "QMessageBox QLabel, QInputDialog QLabel { background-color: transparent; color: #f1f5f9; font-size: 13px; font-weight: 500; }"
      "QMessageBox QLabel#qt_msgbox_label { color: #f1f5f9; font-size: 14px; font-weight: 600; }"
      "QMessageBox QLabel#qt_msgbox_informativelabel { color: #cbd5e1; font-size: 13px; }"
      "QDialog QLabel { background-color: transparent; color: #f1f5f9; }"
      "QMessageBox QPushButton, QInputDialog QPushButton { background: #4f46e5; color: #ffffff; border: 0; border-radius: 8px; padding: 8px 20px; font-weight: 600; font-size: 13px; min-width: 70px; min-height: 24px; }"
      "QMessageBox QPushButton:hover, QInputDialog QPushButton:hover { background: #4338ca; }"
      "QPushButton#slotBtn[slotState='selected'] { background: #4f46e5; color: #ffffff; border: 2px solid #818cf8; border-radius: 10px; font-weight: 700; padding: 8px; font-size: 12px; }"
      "QPushButton#slotBtn[slotState='available'] { background: #151d2d; color: #f8fafc; border: 1.5px solid #334155; border-radius: 10px; font-weight: 600; padding: 8px; font-size: 12px; }"
      "QPushButton#slotBtn[slotState='available']:hover { background: #1e293b; border-color: #818cf8; color: #a5b4fc; }"
      "QPushButton#slotBtn:disabled { background: #080d18; color: #64748b; border: 1px dashed #1e293b; border-radius: 10px; font-weight: 500; padding: 8px; font-size: 12px; }"
    );
  } else {
    setStyleSheet(
      "QMainWindow { background: #f8fafc; }"
      "QWidget { font-family: 'Segoe UI'; font-size: 13px; color: #0f172a; }"
      "QFrame#sidebar { background: #0f172a; border-right: 1px solid #e2e8f0; }"
      "QFrame#brandHeader { background: transparent; border-bottom: 1px solid #1e293b; }"
      "QLabel#brandIcon { background: qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 #4f46e5, stop:1 #7c3aed); border-radius: 12px; }"
      "QLabel#brandTitle { color: #f8fafc; font-size: 18px; font-weight: 700; }"
      "QLabel#brandSubtitle { color: #94a3b8; font-size: 12px; }"
      "QLabel#navSection { color: #64748b; font-size: 11px; font-weight: 700; letter-spacing: 1px; padding: 16px 20px 4px; }"
      "QPushButton#navButton { background: transparent; color: #94a3b8; text-align: left; border: 0; border-radius: 10px; padding: 10px 16px; margin: 2px 12px; font-size: 14px; font-weight: 500; }"
      "QPushButton#navButton:hover { background: #1e293b; color: #f8fafc; }"
      "QPushButton#navButtonActive { background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #4f46e5, stop:1 #6366f1); color: white; text-align: left; border: 0; border-radius: 10px; padding: 10px 16px; margin: 2px 12px; font-size: 14px; font-weight: 700; }"
      "QPushButton#collapseNav { background: #1e293b; color: #f8fafc; border: 0; border-radius: 8px; padding: 0; font-size: 19px; }"
      "QPushButton#collapseNav:hover { background: #334155; }"
      "QFrame#topbar { background: #ffffff; border-bottom: 1px solid #e2e8f0; }"
      "QLabel#pageTitle { font-size: 22px; font-weight: 700; color: #0f172a; }"
      "QLabel#topProfileAvatar { background: qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 #4f46e5, stop:1 #06b6d4); color: white; border-radius: 19px; font-size: 13px; font-weight: 700; }"
      "QLabel#topProfileName { color: #0f172a; font-size: 13px; font-weight: 600; }"
      "QLabel#topProfileRole { color: #64748b; font-size: 11px; }"
      "QStackedWidget { background: #f8fafc; }"
      "QFrame#card, QFrame#formCard, QFrame#settingsCard, QFrame#statCard { background: #ffffff; border: 1px solid #e2e8f0; border-radius: 14px; }"
      "QPushButton { background: #4f46e5; color: white; border: 0; border-radius: 8px; padding: 9px 16px; font-weight: 600; font-size: 13px; }"
      "QPushButton:hover { background: #4338ca; }"
      "QPushButton:disabled { background: #cbd5e1; color: #64748b; }"
      "QPushButton[btnStyle='secondary'] { background: #f1f5f9; color: #334155; border: 1px solid #cbd5e1; }"
      "QPushButton[btnStyle='secondary']:hover { background: #e2e8f0; }"
      "QPushButton[btnStyle='success'] { background: #10b981; color: white; }"
      "QPushButton[btnStyle='success']:hover { background: #059669; }"
      "QPushButton[btnStyle='danger'] { background: #ef4444; color: white; }"
      "QPushButton[btnStyle='danger']:hover { background: #dc2626; }"
      "QPushButton[btnStyle='success']:disabled, QPushButton[btnStyle='danger']:disabled, QPushButton[btnStyle='secondary']:disabled { background: #e2e8f0; color: #94a3b8; border: 1px solid #cbd5e1; }"
      "QLineEdit, QDateEdit, QTimeEdit, QComboBox, QSpinBox {"
      "  background: #ffffff; color: #0f172a; border: 1px solid #cbd5e1; border-radius: 8px; padding: 8px 12px; min-height: 22px; selection-background-color: #6366f1;"
      "}"
      "QLineEdit:focus, QDateEdit:focus, QTimeEdit:focus, QComboBox:focus, QSpinBox:focus { border: 1px solid #6366f1; }"
      "QFrame#availabilityField { background: #f8fafc; border: 1px solid #e2e8f0; border-radius: 12px; }"
      "QLabel#availabilityFieldLabel { color: #17233b; font-size: 13px; font-weight: 700; }"
      "QLabel#availabilityFieldHint { color: #64748b; font-size: 11px; }"
      "QFrame#availabilityField QComboBox, QFrame#availabilityField QDateEdit { background: #ffffff; color: #0f172a; border: 1px solid #cbd5e1; border-radius: 8px; min-height: 30px; padding: 7px 38px 7px 12px; font-size: 13px; }"
      "QFrame#availabilityField QComboBox:focus, QFrame#availabilityField QDateEdit:focus { border: 2px solid #6366f1; }"
      "QFrame#availabilityField QComboBox::drop-down, QFrame#availabilityField QDateEdit::drop-down { width: 30px; border: 0; }"
      "QPushButton#calendarPickerButton { background: #4f46e5; border: 0; border-radius: 9px; padding: 0; }"
      "QPushButton#calendarPickerButton:hover { background: #4338ca; }"
      "QTableWidget { background: #ffffff; color: #0f172a; border: 1px solid #e2e8f0; border-radius: 10px; gridline-color: #f1f5f9; alternate-background-color: #f8fafc; selection-background-color: #e0e7ff; selection-color: #1e1b4b; }"
      "QHeaderView::section { background: #f1f5f9; color: #475569; border: 0; border-bottom: 1px solid #e2e8f0; padding: 10px 12px; font-weight: 600; font-size: 12px; }"
      "QLabel#reportView { background: #ffffff; border: 1px solid #e2e8f0; border-radius: 12px; padding: 24px; color: #0f172a; }"
      "QScrollBar:vertical { background: #f8fafc; width: 10px; border-radius: 5px; }"
      "QScrollBar::handle:vertical { background: #cbd5e1; border-radius: 5px; min-height: 20px; }"
      "QDialog, QMessageBox, QInputDialog { background-color: #ffffff; color: #0f172a; }"
      "QMessageBox QLabel, QInputDialog QLabel { background-color: transparent; color: #0f172a; font-size: 13px; font-weight: 500; }"
      "QMessageBox QLabel#qt_msgbox_label { color: #0f172a; font-size: 14px; font-weight: 600; }"
      "QMessageBox QLabel#qt_msgbox_informativelabel { color: #475569; font-size: 13px; }"
      "QDialog QLabel { background-color: transparent; color: #0f172a; }"
      "QMessageBox QPushButton, QInputDialog QPushButton { background: #4f46e5; color: #ffffff; border: 0; border-radius: 8px; padding: 8px 20px; font-weight: 600; font-size: 13px; min-width: 70px; min-height: 24px; }"
      "QMessageBox QPushButton:hover, QInputDialog QPushButton:hover { background: #4338ca; }"
      "QPushButton#slotBtn[slotState='selected'] { background: #4f46e5; color: #ffffff; border: 2px solid #3730a3; border-radius: 10px; font-weight: 700; padding: 8px; font-size: 12px; }"
      "QPushButton#slotBtn[slotState='available'] { background: #ffffff; color: #0f172a; border: 1.5px solid #cbd5e1; border-radius: 10px; font-weight: 600; padding: 8px; font-size: 12px; }"
      "QPushButton#slotBtn[slotState='available']:hover { background: #f8fafc; border-color: #6366f1; color: #4f46e5; }"
      "QPushButton#slotBtn:disabled { background: #f1f5f9; color: #94a3b8; border: 1px dashed #cbd5e1; border-radius: 10px; font-weight: 500; padding: 8px; font-size: 12px; }"
    );
  }

  QPalette pal = palette();
  if (dark) {
    pal.setColor(QPalette::Window, QColor("#111827"));
    pal.setColor(QPalette::WindowText, QColor("#f1f5f9"));
    pal.setColor(QPalette::Base, QColor("#111827"));
    pal.setColor(QPalette::Text, QColor("#f1f5f9"));
    pal.setColor(QPalette::Button, QColor("#4f46e5"));
    pal.setColor(QPalette::ButtonText, QColor("#ffffff"));
  } else {
    pal.setColor(QPalette::Window, QColor("#ffffff"));
    pal.setColor(QPalette::WindowText, QColor("#0f172a"));
    pal.setColor(QPalette::Base, QColor("#ffffff"));
    pal.setColor(QPalette::Text, QColor("#0f172a"));
    pal.setColor(QPalette::Button, QColor("#4f46e5"));
    pal.setColor(QPalette::ButtonText, QColor("#ffffff"));
  }
  setPalette(pal);
  if (qApp) {
    qApp->setPalette(pal);
    qApp->setStyleSheet(styleSheet());
  }
}

void MainWindow::refreshCurrentPage() {
  if (!m_pages) return;
  QWidget* p = m_pages->currentWidget();
  updateNavBadges();

  if (auto* updates = p->findChild<QTableWidget*>("updatesTable")) {
    updates->setRowCount(0);
    QSet<qint64> notifiedBookings;
    for (const auto& notification : m_db.notificationsForUser(m_user->id())) {
      const int row = updates->rowCount();
      updates->insertRow(row);
      auto* action = new QTableWidgetItem(notification.action);
      action->setData(Qt::UserRole, notification.bookingId);
      updates->setItem(row, 0, action);
      updates->setItem(row, 1, new QTableWidgetItem(QDateTime::fromString(notification.timestamp, Qt::ISODateWithMs).toLocalTime().toString("ddd d MMM yyyy, h:mm AP")));
      updates->setItem(row, 2, new QTableWidgetItem(notification.bookingId > 0 ? QString::number(notification.bookingId) : "Account"));
      updates->setItem(row, 3, new QTableWidgetItem(notification.description));
      if (notification.bookingId > 0) notifiedBookings.insert(notification.bookingId);
    }
    const QVector<Booking> records = m_user->role() == UserRole::Administrator
      ? m_db.allBookings(false) : m_db.bookingsForUser(m_user->id());
    for (const auto& booking : records) {
      if (booking.status == BookingStatus::Pending || notifiedBookings.contains(booking.id)) continue;
      const int row = updates->rowCount();
      updates->insertRow(row);
      auto* action = new QTableWidgetItem("Booking " + toString(booking.status).toLower());
      action->setData(Qt::UserRole, booking.id);
      updates->setItem(row, 0, action);
      updates->setItem(row, 1, new QTableWidgetItem("Earlier"));
      updates->setItem(row, 2, new QTableWidgetItem(booking.reference));
      updates->setItem(row, 3, new QTableWidgetItem(booking.decisionNote.isEmpty() ? "View booking details" : booking.decisionNote));
    }
    QSettings().setValue(QString("updates/seen/%1").arg(m_user->id()), QDateTime::currentDateTimeUtc().toString(Qt::ISODateWithMs));
    updateNavBadges();
    return;
  }

  if (p == m_pages->widget(0)) {
    refreshDashboard();
    return;
  }
  // Users Page Refresh
  if (auto* t = p->findChild<QTableWidget*>("usersTable")) {
    auto* search = p->findChild<QLineEdit*>("usersSearch");
    t->setRowCount(0);
    for (const auto& u : m_db.users(search ? search->text() : QString(), true)) {
      const int r = t->rowCount();
      t->insertRow(r);
      auto* idItem = new QTableWidgetItem(u.eid);
      idItem->setData(Qt::UserRole, u.id);
      t->setItem(r, 0, idItem);
      t->setItem(r, 1, new QTableWidgetItem(u.name));
      t->setItem(r, 2, new QTableWidgetItem(u.email));
      t->setItem(r, 3, new QTableWidgetItem(u.department));
      t->setItem(r, 4, new QTableWidgetItem(toString(u.role)));
      t->setCellWidget(r, 5, createStatusBadgeWidget(u.active ? "Active" : "Inactive", u.active ? "Active" : "Inactive"));
      t->setCellWidget(r, 6, createStatusBadgeWidget(u.approved ? "Approved" : "Pending", u.approved ? "Approved" : "Pending"));
    }
    return;
  }
  // Laboratories Page Refresh
  if (auto* t = p->findChild<QTableWidget*>("laboratoriesTable")) {
    auto* search = p->findChild<QLineEdit*>("labsSearch");
    t->setRowCount(0);
    for (const auto& lab : LaboratoryManager(m_db).search(*m_user, search ? search->text() : QString())) {
      const int r = t->rowCount();
      t->insertRow(r);
      auto* idItem = new QTableWidgetItem(lab.name);
      idItem->setData(Qt::UserRole, lab.id);
      t->setItem(r, 0, idItem);
      t->setItem(r, 1, new QTableWidgetItem(lab.number));
      t->setItem(r, 2, new QTableWidgetItem(lab.location));
      t->setItem(r, 3, new QTableWidgetItem(lab.department));
      t->setItem(r, 4, new QTableWidgetItem(QString::number(lab.capacity)));
      t->setCellWidget(r, 5, createStatusBadgeWidget(toString(lab.status), toString(lab.status)));
      t->setCellWidget(r, 6, createStatusBadgeWidget(lab.active ? "Active" : "Inactive", lab.active ? "Active" : "Inactive"));
    }
    return;
  }
  // Equipment Page Refresh
  if (auto* t = p->findChild<QTableWidget*>("equipmentTable")) {
    auto* search = p->findChild<QLineEdit*>("equipmentSearch");
    QHash<qint64, QString> labs;
    for (const auto& l : m_db.laboratories(true)) labs.insert(l.id, l.name);
    t->setRowCount(0);
    for (const auto& e : EquipmentManager(m_db).search(*m_user, search ? search->text() : QString())) {
      const int r = t->rowCount();
      t->insertRow(r);
      auto* idItem = new QTableWidgetItem(e.name);
      idItem->setData(Qt::UserRole, e.id);
      t->setItem(r, 0, idItem);
      t->setItem(r, 1, new QTableWidgetItem(labs.value(e.labId, "Unknown")));
      t->setItem(r, 2, new QTableWidgetItem(QString::number(e.totalQty)));
      t->setItem(r, 3, new QTableWidgetItem(QString::number(e.availableQty)));
      t->setItem(r, 4, new QTableWidgetItem(e.condition));
      t->setCellWidget(r, 5, createStatusBadgeWidget(toString(e.status), toString(e.status)));
      t->setItem(r, 6, new QTableWidgetItem(e.notes));
      t->setCellWidget(r, 7, createStatusBadgeWidget(e.active ? "Active" : "Inactive", e.active ? "Active" : "Inactive"));
    }
    return;
  }
  // My Bookings Refresh
  if (auto* t = p->findChild<QTableWidget*>("myBookingsTable")) {
    QHash<qint64, QString> labs;
    for (const auto& l : m_db.laboratories(true)) labs.insert(l.id, l.name);
    t->setRowCount(0);
    for (const auto& b : m_db.bookingsForUser(m_user->id())) {
      const int r = t->rowCount();
      t->insertRow(r);
      auto* idItem = new QTableWidgetItem(b.reference);
      idItem->setData(Qt::UserRole, b.id);
      t->setItem(r, 0, idItem);
      t->setItem(r, 1, new QTableWidgetItem(labs.value(b.labId, "Unknown")));
      t->setItem(r, 2, new QTableWidgetItem(b.date.toString("ddd, d MMM yyyy")));
      t->setItem(r, 3, new QTableWidgetItem(b.start.toString("HH:mm") + " - " + b.end.toString("HH:mm")));

      // Equipment summary
      QStringList eqList;
      for (const auto& eq : m_db.equipmentForBooking(b.id)) {
        eqList.append(QString("%1 (x%2)").arg(eq.equipmentName).arg(eq.quantity));
      }
      t->setItem(r, 4, new QTableWidgetItem(eqList.isEmpty() ? "None" : eqList.join(", ")));
      t->setItem(r, 5, new QTableWidgetItem(b.purpose));
      t->setCellWidget(r, 6, createStatusBadgeWidget(toString(b.status), toString(b.status)));
      t->setItem(r, 7, new QTableWidgetItem(b.decisionNote));
    }
    return;
  }
  // Admin Bookings Refresh
  if (auto* t = p->findChild<QTableWidget*>("adminBookingsTable")) {
    QHash<qint64, QString> users, departments, labs;
    for (const auto& u : m_db.users()) { users.insert(u.id, u.name); departments.insert(u.id, u.department); }
    for (const auto& l : m_db.laboratories(true)) labs.insert(l.id, l.name);
    t->setRowCount(0);
    for (const auto& b : m_db.allBookings(false)) {
      const int r = t->rowCount();
      t->insertRow(r);
      auto* idItem = new QTableWidgetItem(b.reference);
      idItem->setData(Qt::UserRole, b.id);
      t->setItem(r, 0, idItem);
      auto* requester = new QTableWidgetItem(users.value(b.userId, "Unknown"));
      requester->setData(Qt::UserRole + 1, departments.value(b.userId));
      t->setItem(r, 1, requester);
      t->setItem(r, 2, new QTableWidgetItem(labs.value(b.labId, "Unknown")));
      auto* dateItem = new QTableWidgetItem(b.date.toString("ddd, d MMM yyyy"));
      dateItem->setData(Qt::UserRole, b.date);
      t->setItem(r, 3, dateItem);
      t->setItem(r, 4, new QTableWidgetItem(b.start.toString("HH:mm") + " - " + b.end.toString("HH:mm")));
      QStringList eqList;
      for (const auto& eq : m_db.equipmentForBooking(b.id)) {
        eqList.append(QString("%1 (x%2)").arg(eq.equipmentName).arg(eq.quantity));
      }
      t->setItem(r, 5, new QTableWidgetItem(eqList.isEmpty() ? "None" : eqList.join(", ")));
      t->setItem(r, 6, new QTableWidgetItem(b.purpose));
      auto* statusItem = new QTableWidgetItem(toString(b.status));
      t->setItem(r, 7, statusItem);
      t->setCellWidget(r, 7, createStatusBadgeWidget(toString(b.status), toString(b.status)));
    }
    auto* search = p->findChild<QLineEdit*>("adminBookingsSearch");
    auto* status = p->findChild<QComboBox*>("adminBookingsStatus");
    auto* lab = p->findChild<QLineEdit*>("adminBookingsLab");
    auto* department = p->findChild<QLineEdit*>("adminBookingsDepartment");
    auto* from = p->findChild<QDateEdit*>("adminBookingsFrom");
    auto* to = p->findChild<QDateEdit*>("adminBookingsTo");
    if (from && to) filterBookingRows(t, search ? search->text() : QString(),
      status && status->currentIndex() > 0 ? status->currentText() : QString(),
      from->date(), to->date(), lab ? lab->text().trimmed() : QString(),
      department ? department->text().trimmed() : QString());
    return;
  }
  // Approvals Table Refresh
  if (auto* t = p->findChild<QTableWidget*>("approvalsTable")) {
    QHash<qint64, QString> users, departments, labs;
    for (const auto& u : m_db.users()) { users.insert(u.id, QString("%1 (%2)").arg(u.name, u.eid)); departments.insert(u.id, u.department); }
    for (const auto& l : m_db.laboratories(true)) labs.insert(l.id, QString("%1 [%2]").arg(l.name, l.number));
    t->setRowCount(0);
    for (const auto& b : m_db.allBookings(true)) {
      const int r = t->rowCount();
      t->insertRow(r);
      auto* ref = new QTableWidgetItem(b.reference);
      ref->setData(Qt::UserRole, b.id);
      t->setItem(r, 0, ref);
      auto* requester = new QTableWidgetItem(users.value(b.userId, "Unknown User"));
      requester->setData(Qt::UserRole + 1, departments.value(b.userId));
      t->setItem(r, 1, requester);
      t->setItem(r, 2, new QTableWidgetItem(labs.value(b.labId, "Unknown Laboratory")));
      auto* dateItem = new QTableWidgetItem(b.date.toString("ddd, d MMM yyyy"));
      dateItem->setData(Qt::UserRole, b.date);
      t->setItem(r, 3, dateItem);
      t->setItem(r, 4, new QTableWidgetItem(b.start.toString("HH:mm") + " - " + b.end.toString("HH:mm")));

      QStringList eqList;
      for (const auto& eq : m_db.equipmentForBooking(b.id)) {
        eqList.append(QString("%1 (x%2)").arg(eq.equipmentName).arg(eq.quantity));
      }
      t->setItem(r, 5, new QTableWidgetItem(eqList.isEmpty() ? "None" : eqList.join(", ")));
      t->setItem(r, 6, new QTableWidgetItem(b.purpose));

      auto* actions = new QWidget;
      auto* al = new QHBoxLayout(actions);
      al->setContentsMargins(4, 2, 4, 2);
      al->setSpacing(8);
      auto* approve = createButton("Approve", "success");
      auto* reject = createButton("Reject", "danger");
      al->addWidget(approve);
      al->addWidget(reject);
      t->setCellWidget(r, 7, actions);

      const auto bookingId = b.id;
      connect(approve, &QPushButton::clicked, this, [this, bookingId] {
        auto result = BookingService(m_db).review(*m_user, bookingId, BookingStatus::Approved, "Approved by administrator");
        if (!result.ok) QMessageBox::warning(this, "Approval Failed", result.error);
        refreshCurrentPage();
        refreshDashboard();
      });
      connect(reject, &QPushButton::clicked, this, [this, bookingId] {
        bool ok = false;
        QString reason = QInputDialog::getText(this, "Reject Reservation", "Reason for rejection (required):", QLineEdit::Normal, QString(), &ok).trimmed();
        if (!ok) return;
        if (reason.isEmpty()) {
          QMessageBox::warning(this, "Reason Required", "Enter a reason so the requester knows what to change.");
          return;
        }
        auto result = BookingService(m_db).review(*m_user, bookingId, BookingStatus::Rejected, reason);
        if (!result.ok) QMessageBox::warning(this, "Rejection Failed", result.error);
        refreshCurrentPage();
        refreshDashboard();
      });
    }
    auto* search = p->findChild<QLineEdit*>("approvalsSearch");
    auto* lab = p->findChild<QLineEdit*>("approvalsLab");
    auto* department = p->findChild<QLineEdit*>("approvalsDepartment");
    auto* from = p->findChild<QDateEdit*>("approvalsFrom");
    auto* to = p->findChild<QDateEdit*>("approvalsTo");
    if (from && to) filterBookingRows(t, search ? search->text() : QString(), {},
      from->date(), to->date(), lab ? lab->text().trimmed() : QString(),
      department ? department->text().trimmed() : QString());
    return;
  }
}

void MainWindow::showBookingDetails(qint64 bookingId) {
  const auto booking = m_db.bookingById(bookingId);
  if (!booking) return;
  QString labName = "Unknown laboratory";
  for (const auto& lab : m_db.laboratories(true)) if (lab.id == booking->labId) labName = lab.name + " [" + lab.number + "]";
  QString requester = "";
  if (const auto user = m_db.userById(booking->userId)) requester = user->name + " (" + user->eid + ")";
  QStringList equipment;
  for (const auto& item : m_db.equipmentForBooking(bookingId))
    equipment << QString("%1 × %2").arg(item.equipmentName.toHtmlEscaped()).arg(item.quantity);
  QString html = QString("<h2>%1</h2><p><b>Requester:</b> %2<br><b>Laboratory:</b> %3<br><b>Date:</b> %4<br><b>Time:</b> %5–%6<br><b>Status:</b> %7<br><b>Purpose:</b> %8<br><b>Equipment:</b> %9</p>")
    .arg(booking->reference.toHtmlEscaped(), requester.toHtmlEscaped(), labName.toHtmlEscaped(),
         booking->date.toString("ddd, d MMM yyyy"), booking->start.toString("HH:mm"), booking->end.toString("HH:mm"),
         toString(booking->status).toHtmlEscaped(), booking->purpose.toHtmlEscaped(),
         equipment.isEmpty() ? "None" : equipment.join(", "));
  if (!booking->decisionNote.isEmpty()) html += "<p><b>Decision note:</b> " + booking->decisionNote.toHtmlEscaped() + "</p>";
  const auto history = m_db.bookingApprovals(bookingId);
  html += "<h3>Decision history</h3>";
  if (history.isEmpty()) html += "<p>No review decisions yet.</p>";
  for (const auto& action : history)
    html += QString("<p><b>%1</b> — %2 by %3<br>%4</p>")
      .arg(action.action.toHtmlEscaped(), action.actionTime.toHtmlEscaped(),
           action.adminName.toHtmlEscaped(), action.comment.toHtmlEscaped());
  QDialog dialog(this);
  dialog.setWindowTitle("Booking " + booking->reference);
  dialog.resize(620, 540);
  auto* layout = new QVBoxLayout(&dialog);
  auto* view = new QTextEdit;
  view->setReadOnly(true);
  view->setHtml(html);
  layout->addWidget(view);
  auto* close = createButton("Close", "primary");
  layout->addWidget(close, 0, Qt::AlignRight);
  connect(close, &QPushButton::clicked, &dialog, &QDialog::accept);
  dialog.exec();
}

// ============================================================================
// Pages
// ============================================================================

QWidget* MainWindow::dashboard() {
  auto* w = new QWidget;
  auto* root = new QVBoxLayout(w);
  root->setContentsMargins(32, 28, 32, 32);
  root->setSpacing(24);

  const bool isAdmin = m_user->role() == UserRole::Administrator;

  // Hero Card
  auto* hero = new QFrame;
  hero->setObjectName("heroCard");
  hero->setStyleSheet("QFrame#heroCard { background: qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 #4338ca, stop:0.55 #4f46e5, stop:1 #7c3aed); border-radius: 16px; border: 0; }");
  auto* heroLayout = new QHBoxLayout(hero);
  heroLayout->setContentsMargins(32, 26, 32, 26);

  auto* welcomeCol = new QVBoxLayout;
  auto* tag = new QLabel("LABBOOKER  /  CAMPUS LABORATORY MANAGEMENT");
  tag->setStyleSheet("color: #c7d2fe; font-size: 11px; font-weight: 700; letter-spacing: 1.2px;");
  auto* welcome = new QLabel("Welcome back, " + m_user->name());
  welcome->setStyleSheet("color: white; font-size: 26px; font-weight: 700; margin-top: 4px;");
  auto* sub = new QLabel(isAdmin
    ? "Review pending reservations, manage laboratory inventory, and generate analytics."
    : "Check laboratory availability, reserve equipment, and track your booking status.");
  sub->setStyleSheet("color: #e0e7ff; font-size: 14px; margin-top: 4px;");

  welcomeCol->addWidget(tag);
  welcomeCol->addWidget(welcome);
  welcomeCol->addWidget(sub);
  heroLayout->addLayout(welcomeCol, 1);

  auto* quickCheck = createButton("Check Availability");
  quickCheck->setStyleSheet("QPushButton { background: white; color: #4338ca; border-radius: 10px; padding: 12px 22px; font-weight: 700; font-size: 14px; } QPushButton:hover { background: #eef2ff; }");
  heroLayout->addWidget(quickCheck, 0, Qt::AlignVCenter);
  connect(quickCheck, &QPushButton::clicked, this, [this] { navigateToPage("Availability"); });

  root->addWidget(hero);

  // 4 KPI Stat Cards
  auto* statsGrid = new QGridLayout;
  statsGrid->setHorizontalSpacing(18);
  statsGrid->setVerticalSpacing(18);

  auto addStatCard = [statsGrid](int col, const QString& title, const QString& val, const QString& hint) {
    auto* card = new QFrame;
    card->setObjectName("statCard");
    auto* cl = new QVBoxLayout(card);
    cl->setContentsMargins(20, 18, 20, 18);
    cl->setSpacing(6);
    auto* h = new QLabel(title);
    h->setStyleSheet("font-size: 12px; font-weight: 700; color: #64748b; letter-spacing: 0.5px;");
    auto* n = new QLabel(val);
    n->setObjectName("dashboardStat" + QString::number(col));
    n->setStyleSheet("font-size: 30px; font-weight: 800; color: #4f46e5;");
    auto* d = new QLabel(hint);
    d->setStyleSheet("font-size: 12px; color: #94a3b8;");
    cl->addWidget(h);
    cl->addWidget(n);
    cl->addWidget(d);
    statsGrid->addWidget(card, 0, col);
  };

  addStatCard(0, isAdmin ? "Upcoming Bookings" : "My Active Bookings", "0", "Scheduled from today onward");
  addStatCard(1, "Available Labs", "0", "Ready for reservations");
  addStatCard(2, isAdmin ? "Pending Approvals" : "My Pending Requests", "0", "Awaiting review");
  addStatCard(3, "Equipment Records", "0", "Across campus laboratories");
  root->addLayout(statsGrid);

  // Recent Bookings Header
  auto* secRow = new QHBoxLayout;
  auto* secTitle = new QLabel(isAdmin ? "Today's & Upcoming Campus Reservations" : "My Upcoming Reservations");
  secTitle->setStyleSheet("font-size: 17px; font-weight: 700;");
  secRow->addWidget(secTitle);
  secRow->addStretch();
  root->addLayout(secRow);

  // Table
  auto* table = new QTableWidget(0, isAdmin ? 6 : 5);
  table->setObjectName("dashboardTable");
  table->setHorizontalHeaderLabels(isAdmin
    ? QStringList{"Requester", "Laboratory", "Date", "Time Window", "Purpose", "Status"}
    : QStringList{"Laboratory", "Date", "Time Window", "Purpose", "Status"});
  configureDataTable(table);
  table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
  table->verticalHeader()->setVisible(false);
  table->setEditTriggers(QAbstractItemView::NoEditTriggers);
  table->setSelectionBehavior(QAbstractItemView::SelectRows);
  table->setAlternatingRowColors(true);
  table->setMinimumHeight(240);
  root->addWidget(table, 1);

  return w;
}

void MainWindow::refreshDashboard() {
  if (!m_pages || m_pages->count() == 0) return;
  QWidget* page = m_pages->widget(0);
  auto* table = page->findChild<QTableWidget*>("dashboardTable");
  if (!table) return;

  const bool admin = (m_user->role() == UserRole::Administrator);
  const auto labs = m_db.laboratories();
  const auto gear = m_db.equipment();
  const auto records = admin ? m_db.allBookings(false) : m_db.bookingsForUser(m_user->id());
  const auto pending = admin ? m_db.allBookings(true) : QVector<Booking>{};

  int available = 0, active = 0, myPending = 0;
  QVector<Booking> upcoming;
  QHash<qint64, QString> labNames, userNames;

  for (const auto& l : labs) {
    labNames.insert(l.id, l.name);
    if (l.status == LaboratoryStatus::Available && l.active) ++available;
  }
  if (admin) {
    for (const auto& u : m_db.users()) userNames.insert(u.id, u.name);
  }

  const QDate today = QDate::currentDate();
  const QTime now = QTime::currentTime();

  for (const auto& b : records) {
    if (!admin && b.status == BookingStatus::Pending) ++myPending;
    if ((b.status == BookingStatus::Pending || b.status == BookingStatus::Approved) &&
        (b.date > today || (b.date == today && b.start > now))) {
      ++active;
      upcoming.push_back(b);
    }
  }

  const QStringList vals = {
    QString::number(active),
    QString::number(available),
    QString::number(admin ? pending.size() : myPending),
    QString::number(gear.size())
  };
  for (int i = 0; i < vals.size(); ++i) {
    if (auto* lbl = page->findChild<QLabel*>("dashboardStat" + QString::number(i))) {
      lbl->setText(vals[i]);
    }
  }

  table->clearSpans();
  table->setRowCount(0);
  std::sort(upcoming.begin(), upcoming.end(), [](const Booking& a, const Booking& b) {
    return a.date == b.date ? a.start < b.start : a.date < b.date;
  });

  for (const auto& b : upcoming) {
    if (table->rowCount() >= 8) break;
    const int r = table->rowCount();
    table->insertRow(r);
    int c = 0;
    if (admin) table->setItem(r, c++, new QTableWidgetItem(userNames.value(b.userId, "Unknown")));
    table->setItem(r, c++, new QTableWidgetItem(labNames.value(b.labId, "Laboratory")));
    table->setItem(r, c++, new QTableWidgetItem(b.date.toString("ddd, d MMM yyyy")));
    table->setItem(r, c++, new QTableWidgetItem(b.start.toString("HH:mm") + " - " + b.end.toString("HH:mm")));
    table->setItem(r, c++, new QTableWidgetItem(b.purpose));
    table->setCellWidget(r, c++, createStatusBadgeWidget(toString(b.status), toString(b.status)));
  }

  if (table->rowCount() == 0) {
    table->insertRow(0);
    auto* empty = new QTableWidgetItem("No upcoming reservations found.");
    empty->setTextAlignment(Qt::AlignCenter);
    table->setItem(0, 0, empty);
    table->setSpan(0, 0, 1, table->columnCount());
  }
}

QWidget* MainWindow::availability() {
  auto* page = new QWidget;
  auto* root = new QVBoxLayout(page);
  root->setContentsMargins(32, 28, 32, 32);
  root->setSpacing(20);

  auto* title = new QLabel("Laboratory Availability Checker");
  title->setStyleSheet("font-size: 20px; font-weight: 700;");
  auto* sub = new QLabel("Select a laboratory and date to compare the four configured two-hour sessions. Weekends and closure dates are unavailable.");
  sub->setStyleSheet("color: #64748b; font-size: 13px;");
  root->addWidget(title);
  root->addWidget(sub);

  auto* card = new QFrame;
  card->setObjectName("formCard");
  auto* cl = new QVBoxLayout(card);
  cl->setContentsMargins(28, 24, 28, 24);
  cl->setSpacing(20);

  auto* formHeading = new QLabel("Find an open laboratory slot");
  formHeading->setStyleSheet("font-size: 17px; font-weight: 700;");
  cl->addWidget(formHeading);

  auto* fields = new QGridLayout;
  fields->setHorizontalSpacing(12);
  fields->setVerticalSpacing(12);

  m_availabilityLabCombo = new QComboBox;

  auto* dateEdit = new QDateEdit(nextOperatingWeekday());
  dateEdit->setMinimumDate(QDate::currentDate());
  dateEdit->setCalendarPopup(false);
  dateEdit->setDisplayFormat("ddd, MMM d, yyyy");

  m_availabilitySlotCombo = new QComboBox;
  for (const auto& s : m_db.bookingSlots()) {
    m_availabilitySlotCombo->addItem(s.fullTitle, s.id);
  }

  int fieldIndex = 0;
  auto addField = [fields, &fieldIndex](const QString& title, QWidget* control, const QString& hint) {
    auto* panel = new QFrame;
    panel->setObjectName("availabilityField");
    auto* column = new QVBoxLayout(panel);
    column->setContentsMargins(16, 14, 16, 13);
    column->setSpacing(7);
    auto* label = new QLabel(title);
    label->setObjectName("availabilityFieldLabel");
    label->setBuddy(control);
    auto* helper = new QLabel(hint);
    helper->setObjectName("availabilityFieldHint");
    helper->setWordWrap(true);
    control->setMinimumHeight(46);
    column->addWidget(label);
    column->addWidget(control);
    column->addWidget(helper);
    fields->addWidget(panel, fieldIndex == 2 ? 1 : 0, fieldIndex == 2 ? 0 : fieldIndex,
                      1, fieldIndex == 2 ? 2 : 1);
    ++fieldIndex;
  };
  addField("Laboratory", m_availabilityLabCombo, "Choose the facility you want to use");
  addField("Date", calendarDateControl(dateEdit), "Monday to Friday only · use the calendar button");
  addField("Time slot", m_availabilitySlotCombo, "Four fixed two-hour sessions");
  cl->addLayout(fields);

  auto* btnRow = new QHBoxLayout;
  auto* checkBtn = createButton("Check Selected Slot", "primary");
  checkBtn->setMinimumWidth(180);
  auto* bookBtn = createButton("Book This Laboratory Now", "success");
  bookBtn->setVisible(false);
  btnRow->addWidget(checkBtn);
  btnRow->addWidget(bookBtn);
  btnRow->addStretch();
  cl->addLayout(btnRow);

  auto* statusBanner = new QLabel("Select laboratory, date, and time slot, then click Check Selected Slot.");
  statusBanner->setStyleSheet("background: #f1f5f9; color: #475569; padding: 12px 16px; border-radius: 8px; font-weight: 600; font-size: 13px;");
  cl->addWidget(statusBanner);
  auto invalidateCheck = [statusBanner, bookBtn] {
    statusBanner->setText("Selection changed. Check the selected slot to continue.");
    statusBanner->setStyleSheet("background: #f1f5f9; color: #475569; padding: 12px 16px; border-radius: 8px; font-weight: 600; font-size: 13px;");
    bookBtn->setVisible(false);
  };
  connect(m_availabilityLabCombo, &QComboBox::currentIndexChanged, this, invalidateCheck);
  connect(dateEdit, &QDateEdit::dateChanged, this, invalidateCheck);
  connect(m_availabilitySlotCombo, &QComboBox::currentIndexChanged, this, invalidateCheck);

  // Existing schedule on that date
  auto* schedTitle = new QLabel("Scheduled Bookings on Selected Date");
  schedTitle->setStyleSheet("font-size: 15px; font-weight: 700; margin-top: 10px;");
  cl->addWidget(schedTitle);

  auto* schedTable = new QTableWidget(0, 4);
  schedTable->setHorizontalHeaderLabels({"Reference", "Time Window", "Purpose", "Status"});
  configureDataTable(schedTable);
  schedTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
  schedTable->verticalHeader()->setVisible(false);
  schedTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
  schedTable->setMinimumHeight(160);
  cl->addWidget(schedTable);

  auto* weekTitle = new QLabel("Week at a glance — double-click an available cell to book");
  weekTitle->setStyleSheet("font-size: 15px; font-weight: 700;");
  cl->addWidget(weekTitle);
  auto* weekTable = new QTableWidget(4, 5);
  weekTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
  weekTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
  weekTable->setSelectionMode(QAbstractItemView::SingleSelection);
  weekTable->setMinimumHeight(230);
  cl->addWidget(weekTable);
  cl->removeWidget(weekTitle);
  cl->removeWidget(weekTable);
  const int scheduleIndex = cl->indexOf(schedTitle);
  cl->insertWidget(scheduleIndex, weekTitle);
  cl->insertWidget(scheduleIndex + 1, weekTable);
  auto updateWeek = [this, dateEdit, weekTable] {
    const QDate monday = dateEdit->date().addDays(1 - dateEdit->date().dayOfWeek());
    QStringList headers;
    for (int col = 0; col < 5; ++col) headers << monday.addDays(col).toString("ddd d MMM");
    weekTable->setHorizontalHeaderLabels(headers);
    const qint64 labId = m_availabilityLabCombo ? m_availabilityLabCombo->currentData().toLongLong() : 0;
    const auto bookingSlots = m_db.bookingSlots();
    QStringList slotLabels;
    for (const auto& slot : bookingSlots) slotLabels << slot.label;
    weekTable->setVerticalHeaderLabels(slotLabels);
    for (int row = 0; row < 4; ++row) for (int col = 0; col < 5; ++col) {
      const QDate day = monday.addDays(col);
      const bool open = labId > 0 && AvailabilityService(m_db).validateRequest(labId, day, bookingSlots[row].start, bookingSlots[row].end).ok;
      auto* item = new QTableWidgetItem(open ? "Available" : "Unavailable");
      item->setTextAlignment(Qt::AlignCenter);
      item->setData(Qt::UserRole, day);
      item->setData(Qt::UserRole + 1, bookingSlots[row].id);
      item->setForeground(QColor(open ? "#065f46" : "#991b1b"));
      item->setBackground(QColor(open ? "#d1fae5" : "#fee2e2"));
      weekTable->setItem(row, col, item);
    }
  };
  connect(m_availabilityLabCombo, &QComboBox::currentIndexChanged, this, updateWeek);
  connect(dateEdit, &QDateEdit::dateChanged, this, updateWeek);
  connect(weekTable, &QTableWidget::cellDoubleClicked, this, [this, weekTable](int row, int col) {
    auto* item = weekTable->item(row, col);
    if (!item || item->text() != "Available") return;
    const auto slot = m_db.bookingSlots()[row];
    prefillBookingForm(m_availabilityLabCombo->currentData().toLongLong(), item->data(Qt::UserRole).toDate(), slot.start, slot.end);
    navigateToPage("New Booking");
  });
  updateWeek();

  auto updateSchedule = [this, dateEdit, schedTable] {
    schedTable->setRowCount(0);
    if (!m_availabilityLabCombo || m_availabilityLabCombo->currentIndex() < 0) return;
    const qint64 labId = m_availabilityLabCombo->currentData().toLongLong();
    if (labId <= 0) {
      schedTable->insertRow(0);
      auto* it = new QTableWidgetItem("No active laboratory selected.");
      schedTable->setItem(0, 0, it);
      schedTable->setSpan(0, 0, 1, 4);
      return;
    }
    const QDate dt = dateEdit->date();
    if (m_db.isClosedDate(dt)) {
      schedTable->insertRow(0);
      auto* it = new QTableWidgetItem("This date is closed. Choose another weekday.");
      schedTable->setItem(0, 0, it);
      schedTable->setSpan(0, 0, 1, 4);
      return;
    }
    for (const auto& b : m_db.allBookings(false)) {
      if (b.labId == labId && b.date == dt && (b.status == BookingStatus::Pending || b.status == BookingStatus::Approved)) {
        const int r = schedTable->rowCount();
        schedTable->insertRow(r);
        schedTable->setItem(r, 0, new QTableWidgetItem(b.reference));
        schedTable->setItem(r, 1, new QTableWidgetItem(b.start.toString("HH:mm") + " - " + b.end.toString("HH:mm")));
        schedTable->setItem(r, 2, new QTableWidgetItem(b.purpose));
        schedTable->setCellWidget(r, 3, createStatusBadgeWidget(toString(b.status), toString(b.status)));
      }
    }
    if (schedTable->rowCount() == 0) {
      schedTable->insertRow(0);
      auto* it = new QTableWidgetItem("No reservations booked for this day yet. Check a slot above before booking.");
      schedTable->setItem(0, 0, it);
      schedTable->setSpan(0, 0, 1, 4);
    }
  };

  updateSchedule();
  connect(m_availabilityLabCombo, &QComboBox::currentIndexChanged, this, updateSchedule);
  connect(dateEdit, &QDateEdit::dateChanged, this, updateSchedule);

  connect(checkBtn, &QPushButton::clicked, this, [this, dateEdit, statusBanner, bookBtn, updateSchedule] {
    updateSchedule();
    if (!m_availabilityLabCombo || m_availabilityLabCombo->currentIndex() < 0) return;
    const qint64 labId = m_availabilityLabCombo->currentData().toLongLong();
    if (labId <= 0) {
      statusBanner->setText("No laboratory selected. Please add or select an active laboratory first.");
      statusBanner->setStyleSheet("background: #fee2e2; color: #991b1b; border: 1px solid #fca5a5; padding: 12px 16px; border-radius: 8px; font-weight: 700; font-size: 13px;");
      bookBtn->setVisible(false);
      return;
    }
    const int slotId = m_availabilitySlotCombo ? m_availabilitySlotCombo->currentData().toInt() : 1;
    const auto allSlots = m_db.bookingSlots();
    if (slotId < 1 || slotId > allSlots.size()) return;
    const auto& s = allSlots[slotId - 1];

    const auto result = AvailabilityService(m_db).validateRequest(labId, dateEdit->date(), s.start, s.end);
    if (result.ok) {
      statusBanner->setText(QString("Available! %1 is open on %2 for %3.")
        .arg(m_availabilityLabCombo->currentText(), dateEdit->date().toString("ddd, MMM d, yyyy"), s.fullTitle));
      statusBanner->setStyleSheet("background: #d1fae5; color: #065f46; border: 1px solid #a7f3d0; padding: 12px 16px; border-radius: 8px; font-weight: 700; font-size: 13px;");
      bookBtn->setVisible(true);
    } else {
      statusBanner->setText("Unavailable: " + result.error);
      statusBanner->setStyleSheet("background: #fee2e2; color: #991b1b; border: 1px solid #fca5a5; padding: 12px 16px; border-radius: 8px; font-weight: 700; font-size: 13px;");
      bookBtn->setVisible(false);
    }
  });

  connect(bookBtn, &QPushButton::clicked, this, [this, dateEdit] {
    if (!m_availabilityLabCombo || !m_availabilitySlotCombo) return;
    const int slotId = m_availabilitySlotCombo->currentData().toInt();
    const auto allSlots = m_db.bookingSlots();
    if (slotId < 1 || slotId > allSlots.size()) return;
    const auto& s = allSlots[slotId - 1];
    prefillBookingForm(m_availabilityLabCombo->currentData().toLongLong(), dateEdit->date(), s.start, s.end);
    navigateToPage("New Booking");
  });

  auto* pageScroll = new QScrollArea;
  pageScroll->setWidgetResizable(true);
  pageScroll->setFrameShape(QFrame::NoFrame);
  pageScroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  pageScroll->setWidget(card);
  root->addWidget(pageScroll, 1);
  return page;
}

QWidget* MainWindow::bookings() {
  auto* w = new QWidget;
  auto* root = new QVBoxLayout(w);
  root->setContentsMargins(32, 28, 32, 32);
  root->setSpacing(22);

  // New Booking Card
  auto* formCard = new QFrame;
  formCard->setObjectName("formCard");
  auto* fl = new QVBoxLayout(formCard);
  fl->setContentsMargins(28, 24, 28, 24);
  fl->setSpacing(16);

  auto* formTitle = new QLabel("Reserve a Laboratory");
  formTitle->setStyleSheet("font-size: 17px; font-weight: 700;");
  fl->addWidget(formTitle);

  auto* grid = new QGridLayout;
  grid->setHorizontalSpacing(24);
  grid->setVerticalSpacing(12);

  m_bookingLabCombo = new QComboBox;
  for (const auto& l : m_db.laboratories()) {
    m_bookingLabCombo->addItem(QString("%1 [%2] — Cap: %3").arg(l.name, l.number).arg(l.capacity), l.id);
  }

  m_bookingDateEdit = new QDateEdit(nextOperatingWeekday());
  m_bookingDateEdit->setMinimumDate(QDate::currentDate());
  m_bookingDateEdit->setCalendarPopup(false);
  m_bookingDateEdit->setDisplayFormat("ddd, MMM d, yyyy");

  m_bookingStartEdit = new QTimeEdit(QTime(8, 30));
  m_bookingStartEdit->setVisible(false);
  m_bookingEndEdit = new QTimeEdit(QTime(10, 30));
  m_bookingEndEdit->setVisible(false);

  auto* purposeEdit = new QLineEdit;
  purposeEdit->setPlaceholderText("Describe the academic or research purpose");
  auto* purposeError = new QLabel;
  purposeError->setStyleSheet("color: #b91c1c; font-weight: 600;");
  purposeError->setVisible(false);

  grid->addWidget(new QLabel("Laboratory"), 0, 0);
  grid->addWidget(m_bookingLabCombo, 1, 0);
  grid->addWidget(new QLabel("Booking Date (Monday to Friday)"), 0, 1);
  grid->addWidget(calendarDateControl(m_bookingDateEdit), 1, 1);
  grid->addWidget(new QLabel("Booking Purpose"), 2, 0, 1, 2);
  grid->addWidget(purposeEdit, 3, 0, 1, 2);
  fl->addLayout(grid);
  fl->addWidget(purposeError);

  // Fixed Time Slots Selector Section
  auto* slotHeaderLayout = new QHBoxLayout;
  auto* slotTitle = new QLabel("Select Fixed Time Slot (2-Hour Sessions)");
  slotTitle->setStyleSheet("font-size: 13px; font-weight: 700; color: #475569; margin-top: 4px;");
  slotHeaderLayout->addWidget(slotTitle);
  slotHeaderLayout->addStretch();
  auto* lunchBadge = new QLabel("Break between morning and afternoon sessions");
  lunchBadge->setStyleSheet("font-size: 11px; font-weight: 600; color: #64748b; background: #f1f5f9; padding: 4px 10px; border-radius: 6px;");
  slotHeaderLayout->addWidget(lunchBadge);
  fl->addLayout(slotHeaderLayout);

  m_bookingSlotNotice = new QLabel;
  m_bookingSlotNotice->setVisible(false);
  fl->addWidget(m_bookingSlotNotice);

  auto* slotsGrid = new QGridLayout;
  slotsGrid->setSpacing(12);
  m_bookingSlotButtons.clear();

  const auto allSlots = m_db.bookingSlots();
  auto editingId = std::make_shared<qint64>(0);

  for (int i = 0; i < allSlots.size(); ++i) {
    const auto& slot = allSlots[i];
    auto* btn = new QPushButton;
    btn->setObjectName("slotBtn");
    btn->setProperty("slotState", "available");
    btn->setCursor(Qt::PointingHandCursor);
    btn->setMinimumHeight(64);
    btn->setProperty("slotId", slot.id);
    m_bookingSlotButtons.append(btn);
    slotsGrid->addWidget(btn, 0, i);

    connect(btn, &QPushButton::clicked, this, [this, i] {
      const auto slot = m_db.bookingSlots().value(i);
      m_selectedSlotId = slot.id;
      if (m_bookingStartEdit) m_bookingStartEdit->setTime(slot.start);
      if (m_bookingEndEdit) m_bookingEndEdit->setTime(slot.end);
      if (m_refreshBookingSlotsFn) m_refreshBookingSlotsFn();
      if (m_refreshEquipmentFn) m_refreshEquipmentFn();
    });
  }
  fl->addLayout(slotsGrid);

  auto updateBookingSlotsUI = [this, editingId] {
    if (!m_bookingLabCombo || !m_bookingDateEdit || m_bookingSlotButtons.isEmpty()) return;
    const qint64 labId = m_bookingLabCombo->currentData().toLongLong();
    const QDate date = m_bookingDateEdit->date();
    const bool weekend = m_db.isClosedDate(date);

    if (weekend) {
      m_bookingSlotNotice->setText("This date is closed. Choose another weekday.");
      m_bookingSlotNotice->setStyleSheet("background: #fee2e2; color: #991b1b; border: 1px solid #fca5a5; padding: 10px 14px; border-radius: 8px; font-weight: 600; font-size: 13px;");
      m_bookingSlotNotice->setVisible(true);
    } else if (labId <= 0) {
      m_bookingSlotNotice->setText("Please select an active laboratory to view available time slots.");
      m_bookingSlotNotice->setStyleSheet("background: #f1f5f9; color: #475569; padding: 10px 14px; border-radius: 8px; font-weight: 600; font-size: 13px;");
      m_bookingSlotNotice->setVisible(true);
    } else {
      m_bookingSlotNotice->setVisible(false);
    }

    const auto allSlots = m_db.bookingSlots();
    int availableCount = 0;
    bool currentSelectedIsAvail = false;

    for (int i = 0; i < allSlots.size() && i < m_bookingSlotButtons.size(); ++i) {
      auto* btn = m_bookingSlotButtons[i];
      const auto& slot = allSlots[i];
      const bool isSelected = (slot.id == m_selectedSlotId);

      bool avail = false;
      QString badge = "Unavailable";
      if (weekend) {
        avail = false;
        badge = "Closed";
      } else if (labId <= 0) {
        avail = false;
        badge = "No Laboratory";
      } else {
        auto res = AvailabilityService(m_db).validateRequest(labId, date, slot.start, slot.end, *editingId);
        avail = res.ok;
        if (!avail) {
          if (res.error.contains("future", Qt::CaseInsensitive)) badge = "Past Slot";
          else if (res.error.contains("reserved", Qt::CaseInsensitive) || res.error.contains("already", Qt::CaseInsensitive)) badge = "Booked";
          else badge = "Unavailable";
        }
      }

      btn->setEnabled(avail);
      if (avail) {
        ++availableCount;
        if (isSelected) {
          currentSelectedIsAvail = true;
          btn->setProperty("slotState", "selected");
          btn->setText(QString("Slot %1 (Selected)\n%2\n[Available]").arg(slot.id).arg(slot.label));
        } else {
          btn->setProperty("slotState", "available");
          btn->setText(QString("Slot %1\n%2\n[Available]").arg(slot.id).arg(slot.label));
        }
      } else {
        btn->setProperty("slotState", "disabled");
        btn->setText(QString("Slot %1\n%2\n[%3]").arg(slot.id).arg(slot.label).arg(badge));
      }
      btn->style()->unpolish(btn);
      btn->style()->polish(btn);
      btn->update();
    }

    if (!currentSelectedIsAvail) {
      for (int i = 0; i < allSlots.size() && i < m_bookingSlotButtons.size(); ++i) {
        if (m_bookingSlotButtons[i]->isEnabled()) {
          m_selectedSlotId = allSlots[i].id;
          if (m_bookingStartEdit) m_bookingStartEdit->setTime(allSlots[i].start);
          if (m_bookingEndEdit) m_bookingEndEdit->setTime(allSlots[i].end);
          m_bookingSlotButtons[i]->setProperty("slotState", "selected");
          m_bookingSlotButtons[i]->setText(QString("Slot %1 (Selected)\n%2\n[Available]").arg(allSlots[i].id).arg(allSlots[i].label));
          m_bookingSlotButtons[i]->style()->unpolish(m_bookingSlotButtons[i]);
          m_bookingSlotButtons[i]->style()->polish(m_bookingSlotButtons[i]);
          m_bookingSlotButtons[i]->update();
          break;
        }
      }
    }

    if (availableCount == 0 && !weekend && labId > 0) {
      m_bookingSlotNotice->setText("All 4 time slots are reserved or unavailable for this laboratory on the selected date.");
      m_bookingSlotNotice->setStyleSheet("background: #fef3c7; color: #92400e; border: 1px solid #fcd34d; padding: 10px 14px; border-radius: 8px; font-weight: 600; font-size: 13px;");
      m_bookingSlotNotice->setVisible(true);
    }
  };

  m_refreshBookingSlotsFn = updateBookingSlotsUI;

  // Equipment Selector Section
  auto* eqTitle = new QLabel("Optional Equipment Reservations");
  eqTitle->setStyleSheet("font-size: 13px; font-weight: 700; color: #475569; margin-top: 6px;");
  fl->addWidget(eqTitle);

  auto* eqContainer = new QWidget;
  auto* eqLayout = new QVBoxLayout(eqContainer);
  eqLayout->setContentsMargins(0, 0, 0, 0);
  eqLayout->setSpacing(8);
  fl->addWidget(eqContainer);

  struct EqRow {
    qint64 id{};
    QCheckBox* check{};
    QSpinBox* spin{};
  };
  auto eqRows = std::make_shared<QVector<EqRow>>();

  auto updateEquipmentList = [this, eqLayout, eqRows] {
    QLayoutItem* item;
    while ((item = eqLayout->takeAt(0)) != nullptr) {
      if (item->widget()) delete item->widget();
      delete item;
    }
    eqRows->clear();

    if (!m_bookingLabCombo || m_bookingLabCombo->currentIndex() < 0) return;
    const qint64 labId = m_bookingLabCombo->currentData().toLongLong();

    int count = 0;
    for (const auto& eq : m_db.equipment()) {
      if (eq.labId == labId && eq.active && eq.status == EquipmentStatus::Available && eq.availableQty > 0) {
        auto* row = new QWidget;
        auto* rl = new QHBoxLayout(row);
        rl->setContentsMargins(0, 0, 0, 0);
        auto* cb = new QCheckBox(QString("%1 (Condition: %2 | Available: %3)").arg(eq.name, eq.condition).arg(eq.availableQty));
        auto* sb = new QSpinBox;
        sb->setRange(1, eq.availableQty);
        sb->setValue(1);
        sb->setEnabled(false);
        connect(cb, &QCheckBox::toggled, sb, &QSpinBox::setEnabled);
        rl->addWidget(cb, 1);
        rl->addWidget(new QLabel("Qty:"));
        rl->addWidget(sb);
        eqLayout->addWidget(row);
        eqRows->append(EqRow{eq.id, cb, sb});
        ++count;
      }
    }
    if (count == 0) {
      auto* none = new QLabel("No specialized equipment registered in this laboratory.");
      none->setStyleSheet("color: #94a3b8; font-style: italic;");
      eqLayout->addWidget(none);
    }
  };

  m_refreshEquipmentFn = updateEquipmentList;
  updateEquipmentList();
  updateBookingSlotsUI();

  connect(m_bookingLabCombo, &QComboBox::currentIndexChanged, this, [updateEquipmentList, updateBookingSlotsUI] {
    updateBookingSlotsUI();
    updateEquipmentList();
  });
  connect(m_bookingDateEdit, &QDateEdit::dateChanged, this, [updateEquipmentList, updateBookingSlotsUI] {
    updateBookingSlotsUI();
    updateEquipmentList();
  });

  auto* btnRow = new QHBoxLayout;
  auto* submitBtn = createButton("Submit Reservation", "primary");
  auto* clearBtn = createButton("Reset Form", "secondary");
  btnRow->addWidget(submitBtn);
  btnRow->addWidget(clearBtn);
  btnRow->addStretch();
  fl->addLayout(btnRow);

  auto* formScroll = new QScrollArea;
  formScroll->setWidgetResizable(true);
  formScroll->setFrameShape(QFrame::NoFrame);
  formScroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  formScroll->setWidget(formCard);
  root->addWidget(formScroll, 1);

  m_myBookingsPage = new QWidget;
  auto* historyLayout = new QVBoxLayout(m_myBookingsPage);
  historyLayout->setContentsMargins(32, 28, 32, 32);
  historyLayout->setSpacing(18);

  // History Table
  auto* histRow = new QHBoxLayout;
  auto* histTitle = new QLabel("My Reservation History");
  histTitle->setStyleSheet("font-size: 17px; font-weight: 700;");
  auto* editBtn = createButton("Edit Pending", "secondary");
  auto* cancelBtn = createButton("Cancel Booking", "danger");
  auto* refreshBtn = createButton("Refresh", "secondary");
  histRow->addWidget(histTitle);
  histRow->addStretch();
  histRow->addWidget(editBtn);
  histRow->addWidget(cancelBtn);
  histRow->addWidget(refreshBtn);
  historyLayout->addLayout(histRow);

  auto* table = new QTableWidget(0, 8);
  table->setObjectName("myBookingsTable");
  table->setHorizontalHeaderLabels({"Reference", "Laboratory", "Date", "Time", "Equipment", "Purpose", "Status", "Decision Note"});
  configureDataTable(table);
  table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
  table->setSelectionBehavior(QAbstractItemView::SelectRows);
  table->setSelectionMode(QAbstractItemView::SingleSelection);
  table->setEditTriggers(QAbstractItemView::NoEditTriggers);
  historyLayout->addWidget(table, 1);
  editBtn->setEnabled(false);
  cancelBtn->setEnabled(false);
  connect(table, &QTableWidget::itemSelectionChanged, this, [this, table, editBtn, cancelBtn] {
    const auto* item = table->currentRow() >= 0 ? table->item(table->currentRow(), 0) : nullptr;
    const auto booking = item ? m_db.bookingById(item->data(Qt::UserRole).toLongLong()) : std::nullopt;
    editBtn->setEnabled(booking && booking->status == BookingStatus::Pending);
    cancelBtn->setEnabled(booking && (booking->status == BookingStatus::Pending || booking->status == BookingStatus::Approved));
  });
  connect(table, &QTableWidget::cellDoubleClicked, this, [this, table](int row, int) {
    if (auto* item = table->item(row, 0)) showBookingDetails(item->data(Qt::UserRole).toLongLong());
  });

  connect(refreshBtn, &QPushButton::clicked, this, [this] { refreshCurrentPage(); });

  connect(submitBtn, &QPushButton::clicked, this, [this, purposeEdit, purposeError, eqRows, editingId, submitBtn, formTitle, updateBookingSlotsUI] {
    purposeError->setVisible(false);
    if (!m_bookingLabCombo || m_bookingLabCombo->currentIndex() < 0 || m_bookingLabCombo->currentData().toLongLong() <= 0) {
      QMessageBox::warning(this, "Booking Error", "Please select a valid active laboratory.");
      return;
    }
    const QDate date = m_bookingDateEdit->date();
    if (m_db.isClosedDate(date)) {
      QMessageBox::warning(this, "Booking Error", "This date is closed. Choose another weekday.");
      return;
    }
    std::optional<FixedBookingSlot> slotOpt;
    for (const auto& slot : m_db.bookingSlots())
      if (slot.start == m_bookingStartEdit->time() && slot.end == m_bookingEndEdit->time()) slotOpt = slot;
    if (!slotOpt) {
      QMessageBox::warning(this, "Booking Error", "Please select one of the four fixed time slots (exactly 2 hours).");
      return;
    }
    if (purposeEdit->text().trimmed().isEmpty()) {
      purposeError->setText("Enter the academic or research purpose.");
      purposeError->setVisible(true);
      purposeEdit->setFocus();
      return;
    }

    BookingRequest r;
    r.labId = m_bookingLabCombo->currentData().toLongLong();
    r.date = date;
    r.start = slotOpt->start;
    r.end = slotOpt->end;
    r.purpose = purposeEdit->text().trimmed();

    for (const auto& row : *eqRows) {
      if (row.check->isChecked()) {
        r.equipment.append({row.id, row.spin->value()});
      }
    }

    const auto lab = m_bookingLabCombo->currentText();
    QStringList equipmentSummary;
    for (const auto& row : *eqRows)
      if (row.check->isChecked()) equipmentSummary << QString("%1 × %2").arg(row.check->text()).arg(row.spin->value());
    const QString summary = QString("Laboratory: %1\nDate: %2\nTime: %3–%4\nEquipment: %5\nPurpose: %6")
      .arg(lab, date.toString("ddd, d MMM yyyy"), slotOpt->start.toString("HH:mm"),
           slotOpt->end.toString("HH:mm"), equipmentSummary.isEmpty() ? "None" : equipmentSummary.join(", "), r.purpose);
    if (QMessageBox::question(this, *editingId ? "Review Booking Changes" : "Review Booking Request",
                              summary + "\n\nContinue?", QMessageBox::Yes | QMessageBox::No) != QMessageBox::Yes) return;

    QString ref;
    const auto res = *editingId
      ? BookingService(m_db).update(*m_user, *editingId, r)
      : BookingService(m_db).create(*m_user, r, &ref);

    if (!res.ok) {
      QMessageBox::warning(this, "Booking Error", res.error);
      return;
    }

    *editingId = 0;
    submitBtn->setText("Submit Reservation");
    formTitle->setText("Reserve a Laboratory");
    purposeEdit->clear();
    if (m_refreshEquipmentFn) m_refreshEquipmentFn();
    refreshDashboard();
    updateBookingSlotsUI();
    navigateToPage("My Bookings");

    if (!ref.isEmpty()) {
      QMessageBox::information(this, "Reservation Submitted",
        QString("Your booking request %1 has been submitted and is pending review.").arg(ref));
    }
  });

  connect(editBtn, &QPushButton::clicked, this, [this, table, purposeEdit, eqRows, editingId, submitBtn, formTitle, updateBookingSlotsUI] {
    if (table->currentRow() < 0) return;
    const qint64 id = table->item(table->currentRow(), 0)->data(Qt::UserRole).toLongLong();
    const auto b = m_db.bookingById(id);
    if (!b || b->status != BookingStatus::Pending) {
      QMessageBox::information(this, "Edit Reservation", "Only pending bookings can be modified.");
      return;
    }
    *editingId = b->id;
    const int idx = m_bookingLabCombo->findData(b->labId);
    if (idx >= 0) m_bookingLabCombo->setCurrentIndex(idx);
    m_bookingDateEdit->setDate(b->date);
    m_bookingStartEdit->setTime(b->start);
    m_bookingEndEdit->setTime(b->end);
    std::optional<FixedBookingSlot> slotOpt;
    for (const auto& slot : m_db.bookingSlots()) if (slot.start == b->start && slot.end == b->end) slotOpt = slot;
    if (slotOpt) {
      m_selectedSlotId = slotOpt->id;
    }
    updateBookingSlotsUI();
    purposeEdit->setText(b->purpose);
    submitBtn->setText("Save Changes");
    formTitle->setText(QString("Edit Reservation: %1").arg(b->reference));
    navigateToPage("New Booking");
    const auto bookedEquipment = m_db.equipmentForBooking(b->id);
    for (const auto& booked : bookedEquipment)
      for (const auto& row : *eqRows) if (row.id == booked.equipmentId) {
        row.check->setChecked(true);
        row.spin->setValue(booked.quantity);
      }
  });

  connect(cancelBtn, &QPushButton::clicked, this, [this, table, updateBookingSlotsUI] {
    if (table->currentRow() < 0) return;
    const qint64 id = table->item(table->currentRow(), 0)->data(Qt::UserRole).toLongLong();
    if (QMessageBox::question(this, "Cancel Booking", "Are you sure you want to cancel this reservation?") != QMessageBox::Yes) return;
    const auto res = BookingService(m_db).cancel(*m_user, id, "Cancelled by user");
    if (!res.ok) {
      QMessageBox::warning(this, "Cancellation Failed", res.error);
      return;
    }
    refreshCurrentPage();
    refreshDashboard();
    updateBookingSlotsUI();
  });

  connect(clearBtn, &QPushButton::clicked, this, [this, editingId, submitBtn, formTitle, purposeEdit, purposeError, updateBookingSlotsUI] {
    *editingId = 0;
    submitBtn->setText("Submit Reservation");
    formTitle->setText("Reserve a Laboratory");
    purposeEdit->clear();
    purposeError->setVisible(false);
    if (m_bookingLabCombo && m_bookingLabCombo->count()) m_bookingLabCombo->setCurrentIndex(0);
    m_bookingDateEdit->setDate(nextOperatingWeekday());
    m_selectedSlotId = 1;
    if (m_refreshEquipmentFn) m_refreshEquipmentFn();
    updateBookingSlotsUI();
  });

  refreshCurrentPage();
  return w;
}

QWidget* MainWindow::approvals() {
  auto* w = new QWidget;
  auto* l = new QVBoxLayout(w);
  l->setContentsMargins(32, 28, 32, 32);
  l->setSpacing(16);

  auto* title = new QLabel("Pending Reservation Approvals Queue");
  title->setStyleSheet("font-size: 20px; font-weight: 700;");
  auto* sub = new QLabel("Review university reservation requests and verify equipment availability before approval.");
  sub->setStyleSheet("color: #64748b; font-size: 13px;");
  l->addWidget(title);
  l->addWidget(sub);

  auto* searchRow = new QHBoxLayout;
  auto* search = new QLineEdit;
  search->setObjectName("approvalsSearch");
  search->setPlaceholderText("Search reference, requester, laboratory or purpose");
  auto* exportBtn = createButton("Export CSV", "secondary");
  searchRow->addWidget(search, 1);
  searchRow->addWidget(exportBtn);
  l->addLayout(searchRow);
  auto* filters = new QHBoxLayout;
  auto* labFilter = new QLineEdit;
  labFilter->setObjectName("approvalsLab");
  labFilter->setPlaceholderText("Filter laboratory");
  auto* departmentFilter = new QLineEdit;
  departmentFilter->setObjectName("approvalsDepartment");
  departmentFilter->setPlaceholderText("Filter department");
  auto* fromFilter = new QDateEdit(QDate(2000, 1, 1));
  fromFilter->setObjectName("approvalsFrom");
  fromFilter->setCalendarPopup(true);
  auto* toFilter = new QDateEdit(QDate(2100, 12, 31));
  toFilter->setObjectName("approvalsTo");
  toFilter->setCalendarPopup(true);
  filters->addWidget(labFilter, 1);
  filters->addWidget(departmentFilter, 1);
  filters->addWidget(new QLabel("From"));
  filters->addWidget(fromFilter);
  filters->addWidget(new QLabel("To"));
  filters->addWidget(toFilter);
  l->addLayout(filters);

  auto* t = new QTableWidget(0, 8);
  t->setObjectName("approvalsTable");
  t->setHorizontalHeaderLabels({"Reference", "Requester", "Laboratory", "Date", "Time Window", "Equipment", "Purpose", "Decisions"});
  configureDataTable(t);
  t->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
  t->setSelectionBehavior(QAbstractItemView::SelectRows);
  t->setEditTriggers(QAbstractItemView::NoEditTriggers);
  l->addWidget(t, 1);
  connect(t, &QTableWidget::cellDoubleClicked, this, [this, t](int row, int) {
    if (auto* item = t->item(row, 0)) showBookingDetails(item->data(Qt::UserRole).toLongLong());
  });
  auto applyFilters = [this] { refreshCurrentPage(); };
  connect(search, &QLineEdit::textChanged, this, applyFilters);
  connect(labFilter, &QLineEdit::textChanged, this, applyFilters);
  connect(departmentFilter, &QLineEdit::textChanged, this, applyFilters);
  connect(fromFilter, &QDateEdit::dateChanged, this, applyFilters);
  connect(toFilter, &QDateEdit::dateChanged, this, applyFilters);
  connect(exportBtn, &QPushButton::clicked, this, [this, t] {
    const QString path = QFileDialog::getSaveFileName(this, "Export Filtered Approvals", "LabBooker-Approvals.csv", "CSV files (*.csv)");
    if (path.isEmpty()) return;
    QFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
      QMessageBox::warning(this, "Export Failed", file.errorString());
      return;
    }
    QTextStream out(&file);
    auto escapeCsv = [](QString value) { value.replace('"', "\"\""); return '"' + value + '"'; };
    out << "Reference,Requester,Department,Laboratory,Date,Time,Equipment,Purpose\n";
    for (int row = 0; row < t->rowCount(); ++row) {
      if (t->isRowHidden(row)) continue;
      QStringList fields;
      for (int col = 0; col < 7; ++col) {
        fields << escapeCsv(t->item(row, col) ? t->item(row, col)->text() : QString());
        if (col == 1) fields << escapeCsv(t->item(row, col)->data(Qt::UserRole + 1).toString());
      }
      out << fields.join(',') << '\n';
    }
    file.close();
    QMessageBox::information(this, "Exported", "Filtered approvals exported to CSV.");
  });

  return w;
}

QWidget* MainWindow::adminBookings() {
  auto* w = new QWidget;
  auto* l = new QVBoxLayout(w);
  l->setContentsMargins(32, 28, 32, 32);
  l->setSpacing(16);

  auto* title = new QLabel("Master Campus Reservations");
  title->setStyleSheet("font-size: 20px; font-weight: 700;");
  l->addWidget(title);

  auto* bar = new QHBoxLayout;
  auto* search = new QLineEdit;
  search->setObjectName("adminBookingsSearch");
  search->setPlaceholderText("Search reference, requester, or laboratory...");
  auto* markComplete = createButton("Mark Completed", "success");
  auto* cancelSelected = createButton("Cancel Booking", "danger");
  auto* refreshBtn = createButton("Refresh", "secondary");
  auto* exportBtn = createButton("Export CSV", "secondary");

  bar->addWidget(search, 1);
  bar->addWidget(markComplete);
  bar->addWidget(cancelSelected);
  bar->addWidget(refreshBtn);
  bar->addWidget(exportBtn);
  l->addLayout(bar);
  auto* filters = new QHBoxLayout;
  auto* statusFilter = new QComboBox;
  statusFilter->setObjectName("adminBookingsStatus");
  statusFilter->addItems({"All statuses", "Pending", "Approved", "Rejected", "Cancelled", "Completed"});
  auto* labFilter = new QLineEdit;
  labFilter->setObjectName("adminBookingsLab");
  labFilter->setPlaceholderText("Filter laboratory");
  auto* departmentFilter = new QLineEdit;
  departmentFilter->setObjectName("adminBookingsDepartment");
  departmentFilter->setPlaceholderText("Filter department");
  auto* fromFilter = new QDateEdit(QDate(2000, 1, 1));
  fromFilter->setObjectName("adminBookingsFrom");
  fromFilter->setCalendarPopup(true);
  auto* toFilter = new QDateEdit(QDate(2100, 12, 31));
  toFilter->setObjectName("adminBookingsTo");
  toFilter->setCalendarPopup(true);
  filters->addWidget(statusFilter);
  filters->addWidget(labFilter);
  filters->addWidget(departmentFilter);
  filters->addWidget(new QLabel("From"));
  filters->addWidget(fromFilter);
  filters->addWidget(new QLabel("To"));
  filters->addWidget(toFilter);
  l->addLayout(filters);

  auto* t = new QTableWidget(0, 8);
  t->setObjectName("adminBookingsTable");
  t->setHorizontalHeaderLabels({"Reference", "Requester", "Laboratory", "Date", "Time", "Equipment", "Purpose", "Status"});
  configureDataTable(t);
  t->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
  t->setSelectionBehavior(QAbstractItemView::SelectRows);
  t->setSelectionMode(QAbstractItemView::SingleSelection);
  t->setEditTriggers(QAbstractItemView::NoEditTriggers);
  l->addWidget(t, 1);
  markComplete->setEnabled(false);
  connect(t, &QTableWidget::cellDoubleClicked, this, [this, t](int row, int) {
    if (auto* item = t->item(row, 0)) showBookingDetails(item->data(Qt::UserRole).toLongLong());
  });
  cancelSelected->setEnabled(false);
  connect(t, &QTableWidget::itemSelectionChanged, this, [this, t, markComplete, cancelSelected] {
    const auto* item = t->currentRow() >= 0 ? t->item(t->currentRow(), 0) : nullptr;
    const auto booking = item ? m_db.bookingById(item->data(Qt::UserRole).toLongLong()) : std::nullopt;
    markComplete->setEnabled(booking && booking->status == BookingStatus::Approved);
    cancelSelected->setEnabled(booking && (booking->status == BookingStatus::Pending || booking->status == BookingStatus::Approved));
  });

  connect(refreshBtn, &QPushButton::clicked, this, [this] { refreshCurrentPage(); });
  auto applyFilters = [this] { refreshCurrentPage(); };
  connect(search, &QLineEdit::textChanged, this, applyFilters);
  connect(statusFilter, &QComboBox::currentIndexChanged, this, applyFilters);
  connect(labFilter, &QLineEdit::textChanged, this, applyFilters);
  connect(departmentFilter, &QLineEdit::textChanged, this, applyFilters);
  connect(fromFilter, &QDateEdit::dateChanged, this, applyFilters);
  connect(toFilter, &QDateEdit::dateChanged, this, applyFilters);
  connect(exportBtn, &QPushButton::clicked, this, [this, t] {
    const QString path = QFileDialog::getSaveFileName(this, "Export Filtered Bookings", "LabBooker-Bookings.csv", "CSV files (*.csv)");
    if (path.isEmpty()) return;
    QFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
      QMessageBox::warning(this, "Export Failed", file.errorString());
      return;
    }
    QTextStream out(&file);
    auto escapeCsv = [](QString value) { value.replace('"', "\"\""); return '"' + value + '"'; };
    out << "Reference,Requester,Department,Laboratory,Date,Time,Equipment,Purpose,Status\n";
    for (int row = 0; row < t->rowCount(); ++row) {
      if (t->isRowHidden(row)) continue;
      QStringList fields;
      for (int col = 0; col < t->columnCount(); ++col) {
        fields << escapeCsv(t->item(row, col) ? t->item(row, col)->text() : QString());
        if (col == 1) fields << escapeCsv(t->item(row, col)->data(Qt::UserRole + 1).toString());
      }
      out << fields.join(',') << '\n';
    }
    file.close();
    QMessageBox::information(this, "Exported", "Filtered bookings exported to CSV.");
  });
  connect(markComplete, &QPushButton::clicked, this, [this, t] {
    if (t->currentRow() < 0) return;
    const qint64 id = t->item(t->currentRow(), 0)->data(Qt::UserRole).toLongLong();
    auto res = BookingService(m_db).review(*m_user, id, BookingStatus::Completed, "Completed successfully");
    if (!res.ok) QMessageBox::warning(this, "Complete Failed", res.error);
    refreshCurrentPage();
    refreshDashboard();
  });
  connect(cancelSelected, &QPushButton::clicked, this, [this, t] {
    if (t->currentRow() < 0) return;
    const qint64 id = t->item(t->currentRow(), 0)->data(Qt::UserRole).toLongLong();
    if (QMessageBox::question(this, "Cancel", "Cancel this booking as administrator?") != QMessageBox::Yes) return;
    auto res = BookingService(m_db).review(*m_user, id, BookingStatus::Cancelled, "Cancelled by administrator");
    if (!res.ok) QMessageBox::warning(this, "Cancel Failed", res.error);
    refreshCurrentPage();
    refreshDashboard();
  });

  return w;
}

QWidget* MainWindow::laboratoriesPage() {
  auto* w = new QWidget;
  auto* l = new QVBoxLayout(w);
  l->setContentsMargins(32, 28, 32, 32);
  l->setSpacing(16);

  auto* title = new QLabel("Laboratory Facility Directory");
  title->setStyleSheet("font-size: 20px; font-weight: 700;");
  l->addWidget(title);

  auto* bar = new QHBoxLayout;
  auto* search = new QLineEdit;
  search->setObjectName("labsSearch");
  search->setPlaceholderText("Search laboratory name, number, location, or department...");
  auto* addBtn = createButton("Add Facility", "primary");
  auto* editBtn = createButton("Edit Selected", "secondary");
  auto* toggleActive = createButton("Toggle Active", "secondary");

  bar->addWidget(search, 1);
  bar->addWidget(addBtn);
  bar->addWidget(editBtn);
  bar->addWidget(toggleActive);
  l->addLayout(bar);

  auto* t = new QTableWidget(0, 7);
  t->setObjectName("laboratoriesTable");
  t->setHorizontalHeaderLabels({"Facility Name", "Room #", "Location", "Department", "Capacity", "Status", "Active"});
  configureDataTable(t);
  t->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
  t->setSelectionBehavior(QAbstractItemView::SelectRows);
  t->setSelectionMode(QAbstractItemView::SingleSelection);
  t->setEditTriggers(QAbstractItemView::NoEditTriggers);
  l->addWidget(t, 1);
  editBtn->setEnabled(false);
  toggleActive->setEnabled(false);
  connect(t, &QTableWidget::itemSelectionChanged, this, [t, editBtn, toggleActive] {
    const bool selected = t->currentRow() >= 0;
    editBtn->setEnabled(selected);
    toggleActive->setEnabled(selected);
  });

  connect(search, &QLineEdit::textChanged, this, [this] { refreshCurrentPage(); });
  connect(addBtn, &QPushButton::clicked, this, [this] { addLaboratoryDialog(); });
  connect(editBtn, &QPushButton::clicked, this, [this, t] {
    if (t->currentRow() < 0) return;
    editLaboratoryDialog(t->item(t->currentRow(), 0)->data(Qt::UserRole).toLongLong());
  });
  connect(toggleActive, &QPushButton::clicked, this, [this, t] {
    if (t->currentRow() < 0) return;
    const qint64 id = t->item(t->currentRow(), 0)->data(Qt::UserRole).toLongLong();
    for (const auto& lab : m_db.laboratories(true)) {
      if (lab.id == id) {
        auto res = lab.active ? LaboratoryManager(m_db).deactivate(*m_user, id) : LaboratoryManager(m_db).activate(*m_user, id);
        if (!res.ok) QMessageBox::warning(this, "Toggle Failed", res.error);
        refreshCurrentPage();
        refreshDashboard();
        refreshLaboratoryCombos();
        break;
      }
    }
  });

  return w;
}

QWidget* MainWindow::equipmentPage() {
  auto* w = new QWidget;
  auto* l = new QVBoxLayout(w);
  l->setContentsMargins(32, 28, 32, 32);
  l->setSpacing(16);

  auto* title = new QLabel("Equipment Inventory");
  title->setStyleSheet("font-size: 20px; font-weight: 700;");
  l->addWidget(title);

  auto* bar = new QHBoxLayout;
  auto* search = new QLineEdit;
  search->setObjectName("equipmentSearch");
  search->setPlaceholderText("Search equipment item, condition, or notes...");
  auto* addBtn = createButton("Add Equipment", "primary");
  auto* editBtn = createButton("Edit Selected", "secondary");
  auto* toggleActive = createButton("Toggle Active", "secondary");

  bar->addWidget(search, 1);
  bar->addWidget(addBtn);
  bar->addWidget(editBtn);
  bar->addWidget(toggleActive);
  l->addLayout(bar);

  auto* t = new QTableWidget(0, 8);
  t->setObjectName("equipmentTable");
  t->setHorizontalHeaderLabels({"Equipment", "Laboratory", "Total Qty", "Available Qty", "Condition", "Status", "Notes", "Active"});
  configureDataTable(t);
  t->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
  t->setSelectionBehavior(QAbstractItemView::SelectRows);
  t->setSelectionMode(QAbstractItemView::SingleSelection);
  t->setEditTriggers(QAbstractItemView::NoEditTriggers);
  l->addWidget(t, 1);

  connect(search, &QLineEdit::textChanged, this, [this] { refreshCurrentPage(); });
  connect(addBtn, &QPushButton::clicked, this, [this] { addEquipmentDialog(); });
  connect(editBtn, &QPushButton::clicked, this, [this, t] {
    if (t->currentRow() < 0) return;
    editEquipmentDialog(t->item(t->currentRow(), 0)->data(Qt::UserRole).toLongLong());
  });
  connect(toggleActive, &QPushButton::clicked, this, [this, t] {
    if (t->currentRow() < 0) return;
    const qint64 id = t->item(t->currentRow(), 0)->data(Qt::UserRole).toLongLong();
    for (const auto& e : m_db.equipment(true)) {
      if (e.id == id) {
        auto res = e.active ? EquipmentManager(m_db).deactivate(*m_user, id) : EquipmentManager(m_db).activate(*m_user, id);
        if (!res.ok) QMessageBox::warning(this, "Toggle Failed", res.error);
        refreshCurrentPage();
        refreshDashboard();
        break;
      }
    }
  });

  return w;
}

QWidget* MainWindow::usersPage() {
  auto* w = new QWidget;
  auto* l = new QVBoxLayout(w);
  l->setContentsMargins(32, 28, 32, 32);
  l->setSpacing(16);

  auto* title = new QLabel("User Management & Approvals");
  title->setStyleSheet("font-size: 20px; font-weight: 700;");
  l->addWidget(title);

  auto* bar = new QHBoxLayout;
  auto* search = new QLineEdit;
  search->setObjectName("usersSearch");
  search->setPlaceholderText("Search by E-ID, full name, or email...");
  auto* addBtn = createButton("Add User", "primary");
  auto* editBtn = createButton("Edit Selected", "secondary");
  auto* approveBtn = createButton("Approve Account", "success");
  auto* toggleActive = createButton("Toggle Active", "secondary");

  bar->addWidget(search, 1);
  bar->addWidget(addBtn);
  bar->addWidget(editBtn);
  bar->addWidget(approveBtn);
  bar->addWidget(toggleActive);
  l->addLayout(bar);

  auto* t = new QTableWidget(0, 7);
  t->setObjectName("usersTable");
  t->setHorizontalHeaderLabels({"E-ID", "Full Name", "Email Address", "Department", "Role", "Active", "Approval Status"});
  configureDataTable(t);
  t->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
  t->setSelectionBehavior(QAbstractItemView::SelectRows);
  t->setSelectionMode(QAbstractItemView::SingleSelection);
  t->setEditTriggers(QAbstractItemView::NoEditTriggers);
  l->addWidget(t, 1);

  connect(search, &QLineEdit::textChanged, this, [this] { refreshCurrentPage(); });
  connect(addBtn, &QPushButton::clicked, this, [this] { addUserDialog(); });
  connect(editBtn, &QPushButton::clicked, this, [this, t] {
    if (t->currentRow() < 0) return;
    editUserDialog(t->item(t->currentRow(), 0)->data(Qt::UserRole).toLongLong());
  });
  connect(approveBtn, &QPushButton::clicked, this, [this, t] {
    if (t->currentRow() < 0) return;
    const qint64 id = t->item(t->currentRow(), 0)->data(Qt::UserRole).toLongLong();
    auto res = UserManager(m_db).approve(*m_user, id);
    if (!res.ok) QMessageBox::warning(this, "Approval Failed", res.error);
    refreshCurrentPage();
  });
  connect(toggleActive, &QPushButton::clicked, this, [this, t] {
    if (t->currentRow() < 0) return;
    const qint64 id = t->item(t->currentRow(), 0)->data(Qt::UserRole).toLongLong();
    const auto u = m_db.userById(id);
    if (!u) return;
    auto res = u->active ? UserManager(m_db).deactivate(*m_user, id) : UserManager(m_db).activate(*m_user, id);
    if (!res.ok) QMessageBox::warning(this, "Toggle Failed", res.error);
    refreshCurrentPage();
  });

  return w;
}

QWidget* MainWindow::reportsPage() {
  auto* w = new QWidget;
  auto* l = new QVBoxLayout(w);
  l->setContentsMargins(32, 28, 32, 32);
  l->setSpacing(18);

  auto* title = new QLabel("System Reports & Analytics");
  title->setStyleSheet("font-size: 20px; font-weight: 700;");
  l->addWidget(title);

  auto* filterCard = new QFrame;
  filterCard->setObjectName("formCard");
  auto* fl = new QHBoxLayout(filterCard);
  fl->setContentsMargins(20, 16, 20, 16);
  fl->setSpacing(14);

  auto* from = new QDateEdit(QDate::currentDate().addMonths(-1));
  from->setCalendarPopup(true);
  auto* to = new QDateEdit(QDate::currentDate());
  to->setCalendarPopup(true);

  auto* genBtn = createButton("Generate", "primary");
  auto* pdfBtn = createButton("Export to PDF", "secondary");
  auto* allTimeBtn = createButton("All Time", "secondary");

  fl->addWidget(new QLabel("From:"));
  fl->addWidget(from);
  fl->addWidget(new QLabel("To:"));
  fl->addWidget(to);
  fl->addWidget(genBtn);
  fl->addWidget(allTimeBtn);
  fl->addStretch();
  fl->addWidget(pdfBtn);
  l->addWidget(filterCard);

  auto* reportView = new QLabel;
  reportView->setObjectName("reportView");
  reportView->setTextFormat(Qt::RichText);
  reportView->setWordWrap(true);
  reportView->setAlignment(Qt::AlignTop | Qt::AlignLeft);

  auto* scroll = new QScrollArea;
  scroll->setWidget(reportView);
  scroll->setWidgetResizable(true);
  scroll->setStyleSheet("border: 0; background: transparent;");
  l->addWidget(scroll, 1);

  auto allTime = std::make_shared<bool>(false);
  auto updateReport = [this, from, to, reportView, allTime] {
    *allTime = false;
    if (from->date() > to->date()) {
      reportView->setText("<p style='color:#dc2626; font-weight:bold;'>Start date must be before end date.</p>");
      return;
    }
    reportView->setText(ReportService(m_db).summaryHtml(from->date(), to->date()));
  };

  connect(genBtn, &QPushButton::clicked, this, updateReport);
  connect(allTimeBtn, &QPushButton::clicked, this, [this, reportView, allTime] {
    *allTime = true;
    reportView->setText(ReportService(m_db).summaryHtml());
  });
  connect(pdfBtn, &QPushButton::clicked, this, [this, from, to, allTime] {
    if (!*allTime && from->date() > to->date()) {
      QMessageBox::warning(this, "Invalid Date Range", "Start date must be before end date.");
      return;
    }
    const QString path = QFileDialog::getSaveFileName(this, "Export PDF Report", "LabBooker-Report.pdf", "PDF files (*.pdf)");
    if (!path.isEmpty()) {
      auto res = *allTime ? ReportService(m_db).exportPdf(path)
                          : ReportService(m_db).exportPdf(path, from->date(), to->date());
      if (!res.ok) QMessageBox::warning(this, "Export Failed", res.error);
      else QMessageBox::information(this, "Exported", "Report has been exported to PDF successfully.");
    }
  });

  updateReport();
  return w;
}

QWidget* MainWindow::profilePage() {
  auto* w = new QWidget;
  auto* root = new QVBoxLayout(w);
  root->setContentsMargins(32, 28, 32, 32);

  auto* card = new QFrame;
  card->setObjectName("formCard");
  card->setMaximumWidth(880);
  auto* cl = new QVBoxLayout(card);
  cl->setContentsMargins(36, 32, 36, 32);
  cl->setSpacing(20);

  auto* title = new QLabel("Account Profile");
  title->setStyleSheet("font-size: 22px; font-weight: 700;");
  auto* sub = new QLabel("Update your user details and profile picture.");
  sub->setStyleSheet("color: #64748b; font-size: 13px;");
  cl->addWidget(title);
  cl->addWidget(sub);

  const auto cur = m_db.userById(m_user->id());
  const UserRecord rec = cur.value_or(UserRecord{});

  auto* idRow = new QHBoxLayout;
  auto* avatar = new QLabel;
  avatar->setFixedSize(80, 80);
  avatar->setAlignment(Qt::AlignCenter);

  auto* idText = new QVBoxLayout;
  auto* dispName = new QLabel(rec.name);
  dispName->setStyleSheet("font-size: 18px; font-weight: 700;");
  auto* dispRole = new QLabel(QString("%1  |  University Member").arg(toString(rec.role)));
  dispRole->setStyleSheet("color: #64748b; font-size: 13px;");
  idText->addWidget(dispName);
  idText->addWidget(dispRole);

  auto* changePhoto = createButton("Upload Photo", "secondary");
  idRow->addWidget(avatar);
  idRow->addSpacing(16);
  idRow->addLayout(idText, 1);
  idRow->addWidget(changePhoto);
  cl->addLayout(idRow);

  auto* grid = new QGridLayout;
  grid->setHorizontalSpacing(24);
  grid->setVerticalSpacing(14);

  auto* nameEdit = new QLineEdit(rec.name);
  auto* emailEdit = new QLineEdit(rec.email);
  auto* deptEdit = new QLineEdit(rec.department);
  auto* eidEdit = new QLineEdit(rec.eid); eidEdit->setReadOnly(true); eidEdit->setEnabled(false);

  grid->addWidget(new QLabel("Full Name"), 0, 0);
  grid->addWidget(nameEdit, 1, 0);
  grid->addWidget(new QLabel("Email Address"), 0, 1);
  grid->addWidget(emailEdit, 1, 1);
  grid->addWidget(new QLabel("Department"), 2, 0);
  grid->addWidget(deptEdit, 3, 0);
  grid->addWidget(new QLabel("University E-ID (Fixed)"), 2, 1);
  grid->addWidget(eidEdit, 3, 1);
  cl->addLayout(grid);

  auto* btnRow = new QHBoxLayout;
  auto* saveBtn = createButton("Save Profile Changes", "primary");
  auto* statusLbl = new QLabel;
  btnRow->addWidget(saveBtn);
  btnRow->addSpacing(16);
  btnRow->addWidget(statusLbl, 1);
  cl->addLayout(btnRow);

  root->addWidget(card, 0, Qt::AlignLeft | Qt::AlignTop);
  root->addStretch();

  const QString imgKey = QString("profiles/%1/image").arg(m_user->id());
  auto selectedImg = std::make_shared<QString>(QSettings().value(imgKey).toString());

  auto paintAvatar = [this, avatar, selectedImg, nameEdit] {
    QPixmap img(80, 80);
    img.fill(Qt::transparent);
    QPainter p(&img);
    p.setRenderHint(QPainter::Antialiasing);
    QPainterPath clip;
    clip.addEllipse(0, 0, 80, 80);
    p.setClipPath(clip);

    QPixmap src(*selectedImg);
    if (!src.isNull()) {
      src = src.scaled(80, 80, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);
      p.drawPixmap(0, 0, src, (src.width() - 80) / 2, (src.height() - 80) / 2, 80, 80);
    } else {
      QLinearGradient g(0, 0, 80, 80);
      g.setColorAt(0, QColor("#4f46e5"));
      g.setColorAt(1, QColor("#06b6d4"));
      p.fillRect(0, 0, 80, 80, g);
      p.setPen(Qt::white);
      QFont f = p.font();
      f.setBold(true);
      f.setPointSize(20);
      p.setFont(f);
      p.drawText(QRect(0, 0, 80, 80), Qt::AlignCenter, userInitials(nameEdit->text()));
    }
    avatar->setPixmap(img);

    if (auto* topAvatar = findChild<QLabel*>("topProfileAvatar")) {
      QPixmap sm(38, 38);
      sm.fill(Qt::transparent);
      QPainter sp(&sm);
      sp.setRenderHint(QPainter::Antialiasing);
      QPainterPath sclip;
      sclip.addEllipse(0, 0, 38, 38);
      sp.setClipPath(sclip);
      if (!src.isNull()) {
        sp.drawPixmap(0, 0, src.scaled(38, 38, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation));
        topAvatar->setText(QString());
        topAvatar->setPixmap(sm);
      } else {
        topAvatar->setPixmap(QPixmap());
        topAvatar->setText(userInitials(nameEdit->text()));
      }
    }
  };

  paintAvatar();
  connect(changePhoto, &QPushButton::clicked, this, [this, selectedImg, paintAvatar, statusLbl] {
    const QString p = QFileDialog::getOpenFileName(this, "Select Profile Picture", {}, "Images (*.png *.jpg *.jpeg *.webp)");
    if (!p.isEmpty()) {
      *selectedImg = p;
      paintAvatar();
      statusLbl->clear();
    }
  });

  connect(saveBtn, &QPushButton::clicked, this, [this, rec, nameEdit, emailEdit, deptEdit, statusLbl, selectedImg, imgKey, dispName] {
    const QString n = nameEdit->text().trimmed();
    const QString em = emailEdit->text().trimmed();
    if (n.isEmpty()) {
      statusLbl->setStyleSheet("color:#dc2626; font-weight:600;");
      statusLbl->setText("Full name cannot be blank.");
      return;
    }
    UserRecord u = rec;
    u.name = n;
    u.email = em;
    u.department = deptEdit->text().trimmed();

    if (!selectedImg->isEmpty() && QFileInfo(*selectedImg).absoluteFilePath() != QFileInfo(QSettings().value(imgKey).toString()).absoluteFilePath()) {
      const QString dir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/profiles";
      QDir().mkpath(dir);
      const QString dest = dir + QString("/user_%1.png").arg(m_user->id());
      QFile::remove(dest);
      if (QFile::copy(*selectedImg, dest)) *selectedImg = dest;
      QSettings().setValue(imgKey, *selectedImg);
    }

    auto res = m_db.updateUser(u);
    if (!res.ok) {
      statusLbl->setStyleSheet("color:#dc2626; font-weight:600;");
      statusLbl->setText(res.error);
      return;
    }
    dispName->setText(n);
    if (auto* topName = findChild<QLabel*>("topProfileName")) topName->setText(n);
    statusLbl->setStyleSheet("color:#10b981; font-weight:600;");
    statusLbl->setText("Profile saved successfully!");
  });

  return w;
}

QWidget* MainWindow::settingsPage() {
  auto* page = new QWidget;
  auto* outer = new QVBoxLayout(page);
  outer->setContentsMargins(0, 0, 0, 0);
  auto* scroll = new QScrollArea;
  scroll->setWidgetResizable(true);
  scroll->setFrameShape(QFrame::NoFrame);
  scroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  outer->addWidget(scroll);
  auto* w = new QWidget;
  scroll->setWidget(w);
  auto* l = new QVBoxLayout(w);
  l->setContentsMargins(32, 28, 32, 32);
  l->setSpacing(20);

  auto* title = new QLabel("System Preferences");
  title->setStyleSheet("font-size: 22px; font-weight: 700;");
  l->addWidget(title);

  // Appearance Theme Card
  auto* themeCard = new QFrame;
  themeCard->setObjectName("settingsCard");
  themeCard->setMaximumWidth(800);
  auto* tl = new QFormLayout(themeCard);
  tl->setContentsMargins(24, 20, 24, 20);
  tl->setVerticalSpacing(12);

  auto* themeCombo = new QComboBox;
  themeCombo->addItems({"Light", "Dark"});
  QSettings pref;
  themeCombo->setCurrentText(pref.value("appearance/theme", "Light").toString());
  tl->addRow("Interface Theme", themeCombo);
  auto* hint = new QLabel("Toggle between Light and Dark interface modes. Changes take effect instantly.");
  hint->setStyleSheet("color: #64748b; font-size: 12px;");
  tl->addRow("", hint);
  l->addWidget(themeCard);

  connect(themeCombo, &QComboBox::currentTextChanged, this, [this](const QString& val) {
    QSettings p;
    p.setValue("appearance/theme", val);
    applyTheme(val);
  });

  if (m_user->role() == UserRole::Administrator) {
    auto* contactCard = new QFrame;
    contactCard->setObjectName("settingsCard");
    contactCard->setMaximumWidth(800);
    auto* contactLayout = new QVBoxLayout(contactCard);
    contactLayout->addWidget(new QLabel("Administrator contact shown on the sign-in screen"));
    auto* contactEdit = new QPlainTextEdit(QSettings().value("university/adminContact").toString());
    contactEdit->setPlaceholderText("Department, email, office, hours and phone");
    contactEdit->setMaximumHeight(100);
    contactLayout->addWidget(contactEdit);
    auto* saveContact = createButton("Save Contact Details", "secondary");
    contactLayout->addWidget(saveContact);
    connect(saveContact, &QPushButton::clicked, this, [contactEdit] {
      QSettings().setValue("university/adminContact", contactEdit->toPlainText().trimmed());
      QMessageBox::information(contactEdit, "Saved", "The sign-in help now shows these contact details.");
    });
    l->addWidget(contactCard);

    auto* scheduleCard = new QFrame;
    scheduleCard->setObjectName("settingsCard");
    scheduleCard->setMaximumWidth(800);
    auto* scheduleLayout = new QFormLayout(scheduleCard);
    scheduleLayout->setContentsMargins(24, 20, 24, 20);
    const auto currentSlots = m_db.bookingSlots();
    auto* morning = new QTimeEdit(currentSlots[0].start);
    morning->setDisplayFormat("h:mm AP");
    auto* afternoon = new QTimeEdit(currentSlots[2].start);
    afternoon->setDisplayFormat("h:mm AP");
    scheduleLayout->addRow("First morning session", morning);
    scheduleLayout->addRow("First afternoon session", afternoon);
    scheduleLayout->addRow(new QLabel("Each session lasts two hours. The second session in each half-day starts immediately after the first."));
    auto* saveSchedule = createButton("Save Booking Schedule", "primary");
    scheduleLayout->addRow(saveSchedule);
    connect(saveSchedule, &QPushButton::clicked, this, [this, morning, afternoon] {
      const auto result = m_db.setBookingSchedule(morning->time(), afternoon->time());
      if (!result.ok) QMessageBox::warning(this, "Schedule Not Saved", result.error);
      else {
        refreshLaboratoryCombos();
        QMessageBox::information(this, "Schedule Saved", "New booking requests now use the updated four sessions.");
      }
    });

    auto* closureDate = new QDateEdit(QDate::currentDate());
    closureDate->setCalendarPopup(true);
    auto* addClosure = createButton("Close This Date", "secondary");
    auto* removeClosure = createButton("Reopen This Date", "secondary");
    auto* closureRow = new QWidget;
    auto* closureLayout = new QHBoxLayout(closureRow);
    closureLayout->setContentsMargins(0, 0, 0, 0);
    closureLayout->addWidget(closureDate);
    closureLayout->addWidget(addClosure);
    closureLayout->addWidget(removeClosure);
    scheduleLayout->addRow("Holiday / closure", closureRow);
    auto* closureList = new QLabel;
    closureList->setWordWrap(true);
    scheduleLayout->addRow("Closed dates", closureList);
    auto refreshClosures = [this, closureList] {
      QStringList dates;
      for (const auto& date : m_db.closedDates()) dates << date.toString("ddd, d MMM yyyy");
      closureList->setText(dates.isEmpty() ? "None" : dates.join(", "));
      refreshLaboratoryCombos();
    };
    connect(addClosure, &QPushButton::clicked, this, [this, closureDate, refreshClosures] {
      const auto result = m_db.setClosedDate(closureDate->date(), true);
      if (!result.ok) QMessageBox::warning(this, "Closure Not Saved", result.error);
      else refreshClosures();
    });
    connect(removeClosure, &QPushButton::clicked, this, [this, closureDate, refreshClosures] {
      const auto result = m_db.setClosedDate(closureDate->date(), false);
      if (!result.ok) QMessageBox::warning(this, "Closure Not Removed", result.error);
      else refreshClosures();
    });
    refreshClosures();
    l->addWidget(scheduleCard);
  }

  // Password Change Card
  auto* pwCard = new QFrame;
  pwCard->setObjectName("settingsCard");
  pwCard->setMaximumWidth(800);
  auto* pl = new QFormLayout(pwCard);
  pl->setContentsMargins(24, 20, 24, 20);
  pl->setVerticalSpacing(14);

  auto* oldPw = new QLineEdit; oldPw->setEchoMode(QLineEdit::Password);
  auto* newPw = new QLineEdit; newPw->setEchoMode(QLineEdit::Password);
  auto* confPw = new QLineEdit; confPw->setEchoMode(QLineEdit::Password);
  auto* changeBtn = createButton("Update Account Password", "primary");
  auto* msg = new QLabel;

  pl->addRow("Current Password", oldPw);
  pl->addRow("New Password (8+ chars)", newPw);
  pl->addRow("Confirm New Password", confPw);
  pl->addRow(changeBtn);
  pl->addRow(msg);
  l->addWidget(pwCard);

  connect(changeBtn, &QPushButton::clicked, this, [this, oldPw, newPw, confPw, msg] {
    if (newPw->text().size() < 8) {
      msg->setStyleSheet("color:#dc2626; font-weight:600;");
      msg->setText("New password must be at least 8 characters long.");
      return;
    }
    if (newPw->text() != confPw->text()) {
      msg->setStyleSheet("color:#dc2626; font-weight:600;");
      msg->setText("Passwords do not match.");
      return;
    }
    auto res = AuthService(m_db).changePassword(m_user->id(), oldPw->text(), newPw->text());
    if (!res.ok) {
      msg->setStyleSheet("color:#dc2626; font-weight:600;");
      msg->setText(res.error);
      return;
    }
    oldPw->clear();
    newPw->clear();
    confPw->clear();
    msg->setStyleSheet("color:#10b981; font-weight:600;");
    msg->setText("Password changed successfully!");
  });

  l->addStretch();
  return page;
}

QWidget* MainWindow::updatesPage() {
  auto* page = new QWidget;
  auto* layout = new QVBoxLayout(page);
  layout->setContentsMargins(32, 28, 32, 32);
  layout->setSpacing(16);
  auto* title = new QLabel("Booking Updates");
  title->setStyleSheet("font-size: 20px; font-weight: 700;");
  layout->addWidget(title);
  layout->addWidget(new QLabel("Approved, rejected, completed and cancelled bookings. Double-click a row for details."));
  auto* table = new QTableWidget(0, 4);
  table->setObjectName("updatesTable");
  table->setHorizontalHeaderLabels({"Update", "When", "Booking ID", "Details"});
  configureDataTable(table);
  table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
  layout->addWidget(table, 1);
  connect(table, &QTableWidget::cellDoubleClicked, this, [this, table](int row, int) {
    if (auto* item = table->item(row, 0)) showBookingDetails(item->data(Qt::UserRole).toLongLong());
  });
  return page;
}

QWidget* MainWindow::simpleTable(const QString& title, const QStringList&) {
  if (title == "Users") return usersPage();
  if (title == "Laboratories") return laboratoriesPage();
  if (title == "Reports") return reportsPage();
  return new QWidget;
}

// ============================================================================
// Dialogs
// ============================================================================

void MainWindow::addLaboratoryDialog() { editLaboratoryDialog(); }

void MainWindow::editLaboratoryDialog(qint64 id) {
  std::optional<Laboratory> original;
  if (id) {
    for (const auto& l : m_db.laboratories(true)) {
      if (l.id == id) { original = l; break; }
    }
  }

  QDialog dialog(this);
  dialog.setWindowTitle(original ? "Edit Facility" : "Add New Facility");
  dialog.setObjectName("facilityDialog");
  dialog.setMinimumWidth(640);
  auto* layout = new QVBoxLayout(&dialog);
  layout->setContentsMargins(0, 0, 0, 0);
  layout->setSpacing(0);

  auto* header = new QFrame;
  header->setObjectName("facilityDialogHeader");
  auto* headerRow = new QHBoxLayout(header);
  headerRow->setContentsMargins(28, 24, 28, 22);
  headerRow->setSpacing(16);
  auto* mark = new QLabel;
  mark->setObjectName("facilityDialogMark");
  mark->setFixedSize(46, 46);
  mark->setAlignment(Qt::AlignCenter);
  mark->setPixmap(navigationIcon("laboratories", QColor("#ffffff")).pixmap(27, 27));
  headerRow->addWidget(mark);
  auto* heading = new QVBoxLayout;
  heading->setSpacing(3);
  auto* title = new QLabel(original ? "Edit laboratory" : "Add laboratory");
  title->setObjectName("facilityDialogTitle");
  auto* subtitle = new QLabel(original ? "Update facility details and operating status."
                                       : "Register a laboratory for campus reservations.");
  subtitle->setObjectName("facilityDialogSubtitle");
  heading->addWidget(title);
  heading->addWidget(subtitle);
  headerRow->addLayout(heading, 1);
  layout->addWidget(header);

  auto* body = new QWidget;
  auto* form = new QGridLayout(body);
  form->setContentsMargins(28, 24, 28, 18);
  form->setHorizontalSpacing(18);
  form->setVerticalSpacing(18);
  form->setColumnStretch(0, 1);
  form->setColumnStretch(1, 1);

  auto* name = new QLineEdit(original ? original->name : "");
  auto* number = new QLineEdit(original ? original->number : "");
  auto* location = new QLineEdit(original ? original->location : "");
  auto* dept = new QLineEdit(original ? original->department : "");
  auto* cap = new QSpinBox; cap->setRange(1, 10000); cap->setValue(original ? original->capacity : 25);
  cap->setButtonSymbols(QAbstractSpinBox::NoButtons);
  auto* desc = new QLineEdit(original ? original->description : "");
  auto* status = new QComboBox; status->addItems({"Available", "Unavailable", "Under Maintenance"});
  if (original) status->setCurrentText(toString(original->status));
  status->setStyleSheet("QComboBox::drop-down { width:0px; border:0; }");
  auto* statusControl = new QWidget;
  statusControl->setFocusProxy(status);
  auto* statusRow = new QHBoxLayout(statusControl);
  statusRow->setContentsMargins(0, 0, 0, 0);
  statusRow->setSpacing(8);
  status->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
  auto* statusMenu = new QPushButton("⌄");
  statusMenu->setObjectName("facilityStatusMenu");
  statusMenu->setFixedSize(44, 44);
  statusMenu->setToolTip("Choose operating status");
  statusMenu->setAccessibleName("Choose operating status");
  statusMenu->setCursor(Qt::PointingHandCursor);
  statusRow->addWidget(status, 1);
  statusRow->addWidget(statusMenu);
  connect(statusMenu, &QPushButton::clicked, status, &QComboBox::showPopup);
  auto* save = createButton(original ? "Save Changes" : "Create Facility");
  save->setObjectName("facilitySaveButton");
  save->setDefault(true);
  name->setPlaceholderText("e.g. Computing Laboratory");
  number->setPlaceholderText("e.g. C-204");
  location->setPlaceholderText("Building or floor");
  dept->setPlaceholderText("Academic department");
  desc->setPlaceholderText("Optional facility notes");

  auto addField = [form](int row, int column, const QString& caption, QWidget* control, int span = 1) {
    auto* field = new QWidget;
    auto* stack = new QVBoxLayout(field);
    stack->setContentsMargins(0, 0, 0, 0);
    stack->setSpacing(7);
    auto* label = new QLabel(caption);
    label->setObjectName("facilityFieldLabel");
    label->setBuddy(control);
    control->setMinimumHeight(44);
    stack->addWidget(label);
    stack->addWidget(control);
    form->addWidget(field, row, column, 1, span);
  };
  addField(0, 0, "Facility name", name);
  addField(0, 1, "Room / lab number", number);
  addField(1, 0, "Location / building", location);
  addField(1, 1, "Department", dept);
  addField(2, 0, "Seating capacity", cap);
  addField(2, 1, "Operating status", statusControl);
  addField(3, 0, "Description", desc, 2);
  layout->addWidget(body);

  auto* footer = new QFrame;
  footer->setObjectName("facilityDialogFooter");
  auto* footerRow = new QHBoxLayout(footer);
  footerRow->setContentsMargins(28, 16, 28, 20);
  footerRow->setSpacing(10);
  auto* cancel = createButton("Cancel", "secondary");
  cancel->setObjectName("facilityCancelButton");
  footerRow->addStretch();
  footerRow->addWidget(cancel);
  footerRow->addWidget(save);
  layout->addWidget(footer);
  connect(cancel, &QPushButton::clicked, &dialog, &QDialog::reject);
  name->setFocus();

  const bool dark = QSettings().value("appearance/theme", "Light").toString() == "Dark";
  dialog.setStyleSheet(dark
    ? "QDialog#facilityDialog { background:#111827; color:#f1f5f9; }"
      "QFrame#facilityDialogHeader { background:#172238; border-bottom:1px solid #334155; }"
      "QFrame#facilityDialogFooter { border-top:1px solid #334155; }"
      "QLabel#facilityDialogTitle { color:#f8fafc; font-size:21px; font-weight:700; }"
      "QLabel#facilityDialogSubtitle { color:#94a3b8; font-size:12px; }"
      "QLabel#facilityFieldLabel { color:#cbd5e1; font-size:12px; font-weight:700; }"
      "QLineEdit,QComboBox,QSpinBox { background:#1e293b; color:#f8fafc; border:1px solid #475569; border-radius:9px; padding:8px 12px; font-size:13px; }"
      "QPushButton#facilityStatusMenu { background:#4f46e5; color:white; border:0; border-radius:9px; padding:0; font-size:20px; font-weight:700; }"
      "QLineEdit:focus,QComboBox:focus,QSpinBox:focus { border:2px solid #818cf8; }"
      "QPushButton#facilitySaveButton { background:#4f46e5; color:white; border:0; border-radius:9px; padding:11px 22px; font-weight:700; }"
      "QPushButton#facilityCancelButton { background:#263449; color:#e2e8f0; border:1px solid #475569; border-radius:9px; padding:10px 20px; font-weight:600; }"
    : "QDialog#facilityDialog { background:#ffffff; color:#0f172a; }"
      "QFrame#facilityDialogHeader { background:#f8fafc; border-bottom:1px solid #e2e8f0; }"
      "QFrame#facilityDialogFooter { border-top:1px solid #e2e8f0; }"
      "QLabel#facilityDialogTitle { color:#17233b; font-size:21px; font-weight:700; }"
      "QLabel#facilityDialogSubtitle { color:#64748b; font-size:12px; }"
      "QLabel#facilityFieldLabel { color:#334155; font-size:12px; font-weight:700; }"
      "QLineEdit,QComboBox,QSpinBox { background:#ffffff; color:#0f172a; border:1px solid #cbd5e1; border-radius:9px; padding:8px 12px; font-size:13px; }"
      "QPushButton#facilityStatusMenu { background:#4f46e5; color:white; border:0; border-radius:9px; padding:0; font-size:20px; font-weight:700; }"
      "QLineEdit:focus,QComboBox:focus,QSpinBox:focus { border:2px solid #6366f1; }"
      "QPushButton#facilitySaveButton { background:#4f46e5; color:white; border:0; border-radius:9px; padding:11px 22px; font-weight:700; }"
      "QPushButton#facilityCancelButton { background:#f8fafc; color:#334155; border:1px solid #cbd5e1; border-radius:9px; padding:10px 20px; font-weight:600; }");
  mark->setStyleSheet("background:#4f46e5; border-radius:11px;");

  connect(save, &QPushButton::clicked, &dialog, [&, original] {
    Laboratory lab = original.value_or(Laboratory{});
    lab.name = name->text().trimmed();
    lab.number = number->text().trimmed();
    lab.location = location->text().trimmed();
    lab.department = dept->text().trimmed();
    lab.capacity = cap->value();
    lab.description = desc->text().trimmed();
    lab.status = status->currentText() == "Unavailable" ? LaboratoryStatus::Unavailable :
                 status->currentText() == "Under Maintenance" ? LaboratoryStatus::UnderMaintenance : LaboratoryStatus::Available;

    auto res = original ? LaboratoryManager(m_db).update(*m_user, lab) : LaboratoryManager(m_db).add(*m_user, lab);
    if (!res.ok) {
      QMessageBox::warning(&dialog, "Save Failed", res.error);
      return;
    }
    dialog.accept();
  });

  if (dialog.exec() == QDialog::Accepted) {
    refreshCurrentPage();
    refreshDashboard();
    refreshLaboratoryCombos();
  }
}

void MainWindow::addUserDialog() { editUserDialog(); }

void MainWindow::editUserDialog(qint64 id) {
  std::optional<UserRecord> original;
  if (id) original = m_db.userById(id);

  QDialog dialog(this);
  dialog.setWindowTitle(original ? "Edit User Record" : "Add Campus User");
  auto* form = new QFormLayout(&dialog);
  styleFormDialog(dialog, *form);

  auto* eid = new QLineEdit(original ? original->eid : "");
  auto* name = new QLineEdit(original ? original->name : "");
  auto* email = new QLineEdit(original ? original->email : "");
  auto* faculty = new QLineEdit(original ? original->faculty : "");
  auto* dept = new QLineEdit(original ? original->department : "");
  auto* contact = new QLineEdit(original ? original->contact : "");
  auto* role = new QComboBox; role->addItems({"Student", "Lecturer", "Administrator"});
  if (original) role->setCurrentText(toString(original->role));
  auto* pw = new QLineEdit; pw->setEchoMode(QLineEdit::Password); pw->setPlaceholderText("Temporary password (8+ chars)");
  auto* save = createButton(original ? "Save Changes" : "Create User");

  form->addRow("University E-ID", eid);
  form->addRow("Full Name", name);
  form->addRow("Email Address", email);
  form->addRow("Faculty", faculty);
  form->addRow("Department", dept);
  form->addRow("Contact Phone", contact);
  form->addRow("Role", role);
  if (!original) form->addRow("Temporary Password", pw);
  form->addRow(save);

  connect(save, &QPushButton::clicked, &dialog, [&, original] {
    UserRecord u = original.value_or(UserRecord{});
    u.eid = eid->text().trimmed();
    u.name = name->text().trimmed();
    u.email = email->text().trimmed();
    u.faculty = faculty->text().trimmed();
    u.department = dept->text().trimmed();
    u.contact = contact->text().trimmed();
    u.role = userRoleFromString(role->currentText());

    auto res = original
      ? UserManager(m_db).update(*m_user, u)
      : UserManager(m_db).add(*m_user, u, pw->text());

    if (!res.ok) {
      QMessageBox::warning(&dialog, "Save Failed", res.error);
      return;
    }
    dialog.accept();
  });

  if (dialog.exec() == QDialog::Accepted) refreshCurrentPage();
}

void MainWindow::addEquipmentDialog() { editEquipmentDialog(); }

void MainWindow::editEquipmentDialog(qint64 id) {
  std::optional<Equipment> original;
  if (id) {
    for (const auto& item : m_db.equipment(true)) {
      if (item.id == id) { original = item; break; }
    }
  }

  const auto labs = m_db.laboratories();
  if (labs.isEmpty()) {
    QMessageBox::information(this, "No Facilities", "Please create an active laboratory before adding equipment.");
    return;
  }

  QDialog dialog(this);
  dialog.setWindowTitle(original ? "Edit Equipment Item" : "Register Equipment");
  auto* form = new QFormLayout(&dialog);
  styleFormDialog(dialog, *form);

  auto* lab = new QComboBox;
  for (const auto& l : labs) lab->addItem(QString("%1 [%2]").arg(l.name, l.number), l.id);
  auto* name = new QLineEdit(original ? original->name : "");
  auto* qty = new QSpinBox; qty->setRange(0, 100000); qty->setValue(original ? original->totalQty : 1);
  auto* avail = new QSpinBox; avail->setRange(0, 100000); avail->setValue(original ? original->availableQty : 1);
  auto* cond = new QLineEdit(original ? original->condition : "Good");
  auto* status = new QComboBox; status->addItems({"Available", "Unavailable", "Maintenance"});
  auto* notes = new QLineEdit(original ? original->notes : "");
  auto* save = createButton(original ? "Save Changes" : "Register Equipment");

  if (original) {
    const int idx = lab->findData(original->labId);
    if (idx >= 0) lab->setCurrentIndex(idx);
    status->setCurrentText(toString(original->status));
  }
  avail->setMaximum(qty->value());
  connect(qty, qOverload<int>(&QSpinBox::valueChanged), avail, [avail](int v) { avail->setMaximum(v); });

  form->addRow("Assigned Facility", lab);
  form->addRow("Equipment Name", name);
  form->addRow("Total Quantity", qty);
  form->addRow("Available Quantity", avail);
  form->addRow("Physical Condition", cond);
  form->addRow("Inventory Status", status);
  form->addRow("Maintenance Notes", notes);
  form->addRow(save);

  connect(save, &QPushButton::clicked, &dialog, [&, original] {
    Equipment eq = original.value_or(Equipment{});
    eq.labId = lab->currentData().toLongLong();
    eq.name = name->text().trimmed();
    eq.totalQty = qty->value();
    eq.availableQty = avail->value();
    eq.condition = cond->text().trimmed();
    eq.notes = notes->text().trimmed();
    eq.status = status->currentText() == "Unavailable" ? EquipmentStatus::Unavailable :
                status->currentText() == "Maintenance" ? EquipmentStatus::Maintenance : EquipmentStatus::Available;

    auto res = original ? EquipmentManager(m_db).update(*m_user, eq) : EquipmentManager(m_db).add(*m_user, eq);
    if (!res.ok) {
      QMessageBox::warning(&dialog, "Save Failed", res.error);
      return;
    }
    dialog.accept();
  });

  if (dialog.exec() == QDialog::Accepted) {
    refreshCurrentPage();
    refreshDashboard();
  }
}
