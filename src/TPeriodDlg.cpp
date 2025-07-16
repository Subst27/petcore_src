#include "TPeriodDlg.h"
#include "TSettings.h"

TPeriodDlg::TPeriodDlg(QWidget *parent) : QDialog(parent)
  {
  setupUi(this);

  okBtn->setShortcut(QKeySequence(Qt::Key_Enter));
  cancelBtn->setShortcut(QKeySequence(Qt::Key_Cancel));

  connect(okBtn,&QPushButton::clicked,this,&TPeriodDlg::accept);
  connect(cancelBtn,&QPushButton::clicked,this,&TPeriodDlg::reject);

  startDateEdit->setMinimumDate(QDate::currentDate());
  startDateEdit->setMaximumDate(QDate::currentDate().addMonths(1));
  startDateEdit->setDate(QDate::currentDate());

  connect(startDateEdit,&QDateEdit::dateChanged,this,[this](const QDate &date) {
    endDateEdit->setMinimumDate(date);;
    });

  endDateEdit->setMinimumDate(QDate::currentDate());
  endDateEdit->setMaximumDate(QDate::currentDate().addMonths(1));
  endDateEdit->setDate(QDate::currentDate().addDays(6));

  readSettings();
  }

QPair<QDate, QDate> TPeriodDlg::period() const
  {
  return QPair<QDate,QDate> {startDateEdit->date(),endDateEdit->date()};
  }

void TPeriodDlg::writeSettings()
  {
  TSettings().setXmlValue("period_dialog/geometry","",saveGeometry());
  }

void TPeriodDlg::readSettings()
  {
  restoreGeometry(TSettings().getXmlValue("period_dialog/geometry","",QByteArray()).toByteArray());
  }

void TPeriodDlg::accept()
  {
  writeSettings();
  QDialog::accept();
  }

void TPeriodDlg::reject()
  {
  writeSettings();
  QDialog::reject();
  }
