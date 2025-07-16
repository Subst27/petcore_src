#include "TAboutDlg.h"
#include "TSettings.h"

#include <QDebug>

#include <QDesktopServices>
#include <QUrl>

TAboutDlg::TAboutDlg(QWidget *parent) : QDialog(parent)
  {
  setupUi(this);

  connect(okBtn,&QPushButton::clicked,this,&TAboutDlg::reject);

  connect(siteLbl,&QLabel::linkActivated,siteLbl,[this](const QString &link){
    QDesktopServices::openUrl(QUrl(link));
    });

  connect(emailLbl,&QLabel::linkActivated,emailLbl,[this](const QString &link){
    QDesktopServices::openUrl(QUrl(link));
    });

  readSettings();
  }

void TAboutDlg::showEvent(QShowEvent *event)
  {
  QDialog::showEvent(event);

  QImage image=QImage(":/images/logo_no_text").scaled(logoLbl->size(),Qt::KeepAspectRatio);
  logoLbl->setPixmap(QPixmap::fromImage(image));
  }

void TAboutDlg::resizeEvent(QResizeEvent *event)
  {
  QDialog::resizeEvent(event);

  QImage image=QImage(":/images/logo_no_text").scaled(logoLbl->size(),Qt::KeepAspectRatio);
  logoLbl->setPixmap(QPixmap::fromImage(image));
  }

void TAboutDlg::writeSettings()
  {
  TSettings().setXmlValue("about_dialog/geometry","",saveGeometry());
  }

void TAboutDlg::readSettings()
  {
  restoreGeometry(TSettings().getXmlValue("about_dialog/geometry","",QByteArray()).toByteArray());
  }

void TAboutDlg::reject()
  {
  writeSettings();
  QDialog::reject();
  }
