#include "mainwindow.h"
#include <QApplication>
#include <QIcon>
#include <QPixmap>
#include <QRect>
#include <QStandardPaths>
#include <QDir>
#include <QMessageBox>
#include <QDialog>
#include <QFormLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QStackedWidget>
#include <QStyleFactory>

int main(int argc,char**argv){
  QApplication app(argc,argv);
  app.setStyle(QStyleFactory::create("Fusion"));
  // Keep the established application identity so existing LabBooker records remain in their current data folder.
  app.setApplicationName("LabBooker");
  app.setOrganizationName("Team 13");
  QPalette appPal = app.palette();
  appPal.setColor(QPalette::Window, QColor("#ffffff"));
  appPal.setColor(QPalette::WindowText, QColor("#0f172a"));
  appPal.setColor(QPalette::Base, QColor("#ffffff"));
  appPal.setColor(QPalette::Text, QColor("#0f172a"));
  appPal.setColor(QPalette::Button, QColor("#4f46e5"));
  appPal.setColor(QPalette::ButtonText, QColor("#ffffff"));
  app.setPalette(appPal);
  const QPixmap logo(":/brand/labbooker.png");
  if(!logo.isNull()){const QPixmap mark=logo.copy(QRect(145,54,310,210));app.setWindowIcon(QIcon(mark));}
  const QString dataPath=QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
  if(!QDir().mkpath(dataPath)){QMessageBox::critical(nullptr,"LabBook","Cannot create the application data folder.");return 1;}
  DatabaseManager db(dataPath+"/labbooker.sqlite");
  const auto opened=db.open();
  if(!opened.ok){QMessageBox::critical(nullptr,"Database error",opened.error);return 1;}

  if(!db.hasUsers()){
    QDialog setup;setup.setWindowTitle("First-run administrator setup");setup.setMinimumWidth(520);setup.setStyleSheet("QDialog{background:#ffffff;color:#172033} QDialog QLabel{color:#344054;font-size:13px} QDialog QLineEdit{background:#f8fafc;color:#101828;border:1px solid #d5dce8;border-radius:8px;padding:9px 11px} QDialog QPushButton{background:#5145ed;color:white;border:0;border-radius:8px;padding:10px 16px;font-weight:600}");
    auto*form=new QFormLayout(&setup);form->setContentsMargins(26,24,26,24);form->setHorizontalSpacing(22);form->setVerticalSpacing(14);auto*eid=new QLineEdit;auto*name=new QLineEdit;
    auto*password=new QLineEdit;password->setEchoMode(QLineEdit::Password);
    form->addRow("Administrator E-ID",eid);form->addRow("Full name",name);form->addRow("Password (8+ characters)",password);
    auto*create=new QPushButton("Create administrator");form->addRow(create);
    QObject::connect(create,&QPushButton::clicked,&setup,[&]{
      UserRecord user;user.eid=eid->text().trimmed();user.name=name->text().trimmed();user.role=UserRole::Administrator;
      const auto result=AuthService(db).bootstrap(user,password->text());
      if(!result.ok){QMessageBox::warning(&setup,"Setup failed",result.error);return;}setup.accept();
    });
    if(setup.exec()!=QDialog::Accepted)return 0;
  }

  QStackedWidget shell;
  shell.setWindowTitle("LabBooker - University Laboratory Booking System");
  shell.resize(1240,800);
  auto*login=new LoginWindow(db);
  shell.addWidget(login);
  QObject::connect(login,&LoginWindow::authenticated,&shell,[&](std::shared_ptr<User>user){
    auto*main=new MainWindow(db,std::move(user),&shell);
    shell.addWidget(main);shell.setCurrentWidget(main);
    QObject::connect(main,&MainWindow::logoutRequested,&shell,[&,main]{
      login->reset();shell.setCurrentWidget(login);main->deleteLater();
    });
  });
  shell.setCurrentWidget(login);shell.show();
  return app.exec();
}
