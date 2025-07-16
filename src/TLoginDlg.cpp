#include "TLoginDlg.h"
#include "TSettings.h"

#include <QDebug>
#include <QRegularExpressionValidator>
#include <QMessageBox>

TLoginDlg::TLoginDlg(QWidget *parent) : QDialog(parent)
  {
  setupUi(this);

  connect(loginEdit,&QLineEdit::cursorPositionChanged,this,&TLoginDlg::loginPositionChanged);
  connect(echoModeBtn,&QPushButton::clicked,this,&TLoginDlg::changeEchoMode);

  okBtn->setShortcut(QKeySequence(Qt::Key_Enter));
  cancelBtn->setShortcut(QKeySequence(Qt::Key_Cancel));

  connect(okBtn,&QPushButton::clicked,this,&TLoginDlg::accept);
  connect(cancelBtn,&QPushButton::clicked,this,&TLoginDlg::reject);

  loginEdit->setFocus();
  readSettings();
  }

QPair<QString,QString> TLoginDlg::credentials() const
  {
  return QPair<QString,QString>(loginEdit->text(),passwordEdit->text());
  }

void TLoginDlg::showEvent(QShowEvent *event)
  {
  QDialog::showEvent(event);

  QImage image=QImage(":/images/logo_no_text").scaled(logoLbl->size(),Qt::KeepAspectRatio);
  logoLbl->setPixmap(QPixmap::fromImage(image));
  }

void TLoginDlg::resizeEvent(QResizeEvent *event)
  {
  QDialog::resizeEvent(event);

  QImage image=QImage(":/images/logo_no_text").scaled(logoLbl->size(),Qt::KeepAspectRatio);
  logoLbl->setPixmap(QPixmap::fromImage(image));
  }

void TLoginDlg::writeSettings()
  {
  TSettings().setXmlValue("login_dialog/geometry","",saveGeometry());
  }

void TLoginDlg::readSettings()
  {
  restoreGeometry(TSettings().getXmlValue("login_dialog/geometry","",QByteArray()).toByteArray());
  }

void TLoginDlg::changeEchoMode(bool checked)
  {
  if (checked==true)
    {
    // NOTE: достать префик темы из qApp->property("appt_theme").toString()
    echoModeBtn->setIcon(QPixmap(":/images/eye_off"));
    passwordEdit->setEchoMode(QLineEdit::Normal);
    echoModeBtn->setToolTip(tr("Hide password"));
    }
  else
    {
    // NOTE: достать префик темы из qApp->property("appt_theme").toString()
    echoModeBtn->setIcon(QPixmap(":/images/eye_on"));
    passwordEdit->setEchoMode(QLineEdit::Password);
    echoModeBtn->setToolTip(tr("Show password"));
    }
  }

void TLoginDlg::loginPositionChanged(int oldPos, int newPos)
  {
  Q_UNUSED(oldPos)
  Q_UNUSED(newPos)

  quint16 position=loginEdit->cursorPosition();
  quint16 size=loginEdit->text().size();
  if (position>size)
    loginEdit->setCursorPosition(size);
  }

void TLoginDlg::accept()
  {
  QRegularExpressionValidator validator(QRegularExpression("^[A-Za-z0-9._%+-]+@[A-Za-z0-9.-]+\\.[A-Za-z]{2,}$")); // email address
  //QRegularExpressionValidator validator(QRegularExpression("^\\+7[0-9]{10}$")); // cell phone number in russian region (+7)
  loginEdit->setValidator(&validator);
  if (loginEdit->hasAcceptableInput()==false)
    {
    QMessageBox::critical(this,tr("Error"),tr("Invalid email address."),QMessageBox::Ok);
    loginEdit->setFocus();
    return;
    }

  writeSettings();
  QDialog::accept();
  }

void TLoginDlg::reject()
  {
  writeSettings();
  QDialog::reject();
  }

