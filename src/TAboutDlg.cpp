#include "TAboutDlg.h"

#include <QDesktopServices>
#include <QUrl>

TAboutDlg::TAboutDlg(QWidget *parent) : QDialog(parent)
  {
  setupUi(this);

  connect(okBtn,&QPushButton::clicked,this,&TAboutDlg::accept);

  connect(amteoSiteLbl,&QLabel::linkActivated,amteoSiteLbl,[this](const QString &link){
    QDesktopServices::openUrl(QUrl(link));
    });

  connect(emailLbl,&QLabel::linkActivated,emailLbl,[this](const QString &link){
    QDesktopServices::openUrl(QUrl(link));
    });
  }
