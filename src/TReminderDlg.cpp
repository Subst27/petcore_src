#include "TReminderDlg.h"
#include "TDataModule.h"

#include <QMessageBox>
#include <QDebug>

TReminderDlg::TReminderDlg(const QString &clientName, QWidget *parent) : TModelDlg(QSqlRecord(), "reminder_dialog", parent)
  {
  setupUi(this);

  setWindowTitle(tr("New reminder for '%1'").arg(clientName));

  okBtn->setShortcut(QKeySequence(Qt::Key_Enter));
  cancelBtn->setShortcut(QKeySequence(Qt::Key_Cancel));

  connect(okBtn,&QPushButton::clicked,this,&TReminderDlg::accept);
  connect(cancelBtn,&QPushButton::clicked,this,&TReminderDlg::reject);

  reminderDateEdit->setDate(QDate::currentDate().addDays(1));
  reminderDateEdit->setFocus();

  readSettings();
  }

QSqlRecord TReminderDlg::record() const
  {
  QSqlRecord record=dataModule()->tableRecord("reminders");

  record.setValue("reminder_date", reminderDateEdit->date().toString("yyyy-MM-dd"));
  record.setValue("reminder", reminderEdit->text());

  return record;
  }

void TReminderDlg::accept()
  {
  if (reminderDateEdit->date()<=QDate::currentDate())
    {
    QMessageBox::critical(this,tr("Error"),tr("Reminder date must be at least tomorrow."),QMessageBox::Ok);
    reminderDateEdit->setFocus();
    return;
    }

  if (reminderEdit->text().isEmpty()==true)
    {
    QMessageBox::critical(this,tr("Error"),tr("Reminder can't be empty."),QMessageBox::Ok);
    reminderEdit->setFocus();
    return;
    }

  TModelDlg::accept();
  }
