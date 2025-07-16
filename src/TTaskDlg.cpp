#include "TTaskDlg.h"
#include "TDataModule.h"

#include <QMessageBox>
#include <QDebug>

TTaskDlg::TTaskDlg(const QSqlRecord &record, QWidget *parent) : TModelDlg(record, "reminder_dialog", parent)
  {
  setupUi(this);

  QString clientName=record.value("client_name").toString();
  m_task=clientName.isEmpty();

  if (m_task==true) // это Задача
    {
    setWindowTitle(tr("New task"));

    dateLbl->setText(tr("Task date:"));
    textLbl->setText(tr("Task text"));

    dateEdit->setDate(record.value("task_date").toDate());
    textEdit->setText(record.value("task").toString());
    }
  else // Это Напоминание
    {
    setWindowTitle(tr("New reminder for '%1'").arg(clientName));

    dateLbl->setText(tr("Reminder date:"));
    textLbl->setText(tr("Reminder text"));

    dateEdit->setDate(record.value("reminder_date").toDate());
    textEdit->setText(record.value("reminder").toString());
    }

  okBtn->setShortcut(QKeySequence(Qt::Key_Enter));
  cancelBtn->setShortcut(QKeySequence(Qt::Key_Cancel));

  connect(okBtn,&QPushButton::clicked,this,&TTaskDlg::accept);
  connect(cancelBtn,&QPushButton::clicked,this,&TTaskDlg::reject);

  textEdit->setFocus();

  readSettings();
  }

QSqlRecord TTaskDlg::record() const
  {
  QSqlRecord record;
  if (m_task==true)
    {
    record=dataModule()->tableRecord("tasks");
    record.setValue("task_date",dateEdit->date().toString("yyyy-MM-dd"));
    record.setValue("task",textEdit->text());
    record.setValue("completed",false);
    }
  else
    {
    record=dataModule()->tableRecord("reminders");
    record.setValue("reminder_date",dateEdit->date().toString("yyyy-MM-dd"));
    record.setValue("reminder",textEdit->text());
    }

  return record;
  }

void TTaskDlg::accept()
  {
  if (m_task==true && dateEdit->date()<QDate::currentDate())
    {
    QMessageBox::critical(this,tr("Error"),tr("Task date must be at least today."),QMessageBox::Ok);
    dateEdit->setFocus();
    return;
    }

  if (m_task==false && dateEdit->date()<=QDate::currentDate())
    {
    QMessageBox::critical(this,tr("Error"),tr("Reminder date must be at least tomorrow."),QMessageBox::Ok);
    dateEdit->setFocus();
    return;
    }

  if (textEdit->text().isEmpty()==true)
    {
    QMessageBox::critical(this,tr("Error"),tr("Text can't be empty."),QMessageBox::Ok);
    textEdit->setFocus();
    return;
    }

  TModelDlg::accept();
  }
