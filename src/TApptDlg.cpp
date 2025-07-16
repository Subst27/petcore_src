#include "TApptDlg.h"

#include "TActionsModel.h"
#include "TDoctorsModel.h"
#include "TClientsModel.h"
#include "TPetsModel.h"

#include "TRoutinesModel.h"
#include "TRoutinesFilterModel.h"

#include "TApptsModel.h"
#include "TApptsFilterModel.h"

#include "TPeriodDlg.h"

#include <QMessageBox>

TApptDlg::TApptDlg(TMainWnd::PageTabs pageTab, const QSqlRecord &record, QWidget *parent) : TModelDlg(record, "appt_dialog", parent), m_pageTab(pageTab)
  {
  setupUi(this);

  QDateTime dateTime(record.value("appt_date").toDate(), record.value("appt_time").toTime());
  setWindowTitle(tr("Appointment [%1]").arg(dateTime.isValid()==true ? dateTime.toString("dd.MM.yyyy hh:mm") : tr("Unknown date-time")));

  if (dateTime.isValid()==true)
    {
    QStringList weekDays={tr("Monday"),tr("Tuesday"),tr("Wednesday"),tr("Thursday"),tr("Friday"),tr("Saturday"),tr("Sunday")};
    QString text=QString("%1 (%2) %3").arg(dateTime.toString("dd.MM.yyyy"),weekDays.at(dateTime.date().dayOfWeek()-1),dateTime.toString("hh:mm"));
    apptDateTimeCombo->addItem(text, dateTime);
    }

  TActionsModel *actionsModel=qobject_cast<TActionsModel*>(dataModule()->tableModel("actions"));
  actionCombo->setModel(actionsModel,{"action"},"id");
  actionCombo->setCurrentIndex(actionCombo->findData(record.value("action_id")));

  TDoctorsModel *doctorsModel=qobject_cast<TDoctorsModel*>(dataModule()->tableModel("doctors"));
  // взять, если установлен, существующий фильтр и добавить признак доступности
  QStringList filterParts;
  if (doctorsModel->filter().isEmpty()==false)
    filterParts << doctorsModel->filter();
  // для m_pageTab=Undefined не добавлять available=true, UndefinedTab передается для отчетов
  if (pageTab!=TMainWnd::UndefinedTab)
    filterParts << "doctors.available=true";

  doctorCombo->setModel(doctorsModel,{"name","profile","action_id"},"id",filterParts.join(" and "));
  connect(doctorCombo,qOverload<int>(&TComboBox::currentIndexChanged),this,&TApptDlg::doctorChanged);
  doctorCombo->setCurrentIndex(doctorCombo->findData(record.value("doctor_id")));

  TClientsModel *clientsModel=qobject_cast<TClientsModel*>(dataModule()->tableModel("clients"));
  clientCombo->setModel(clientsModel,{"name","phone_number","telegram"},"id");
  connect(clientCombo,qOverload<int>(&TComboBox::currentIndexChanged),this,&TApptDlg::clientChanged);
  clientCombo->setCurrentIndex(clientCombo->findData(record.value("client_id")));

  connect(petCombo,qOverload<int>(&TComboBox::currentIndexChanged),this,&TApptDlg::petChanged);
  petCombo->setCurrentIndex(petCombo->findData(record.value("pet_id")));

  routineBtn->setEnabled(doctorCombo->currentIndex()>-1);
  editDoctorBtn->setEnabled(doctorCombo->currentIndex()>-1);

  editClientBtn->setEnabled(clientCombo->currentIndex()>-1);
  editPetBtn->setEnabled(petCombo->currentIndex()>-1);

  connect(appendDoctorBtn,&QPushButton::clicked,this,&TApptDlg::appendDoctor);
  connect(editDoctorBtn,&QPushButton::clicked,this,&TApptDlg::editDoctor);

  connect(appendClientBtn,&QPushButton::clicked,this,&TApptDlg::appendClient);
  connect(editClientBtn,&QPushButton::clicked,this,&TApptDlg::editClient);

  connect(appendPetBtn,&QPushButton::clicked,this,&TApptDlg::appendPet);
  connect(editPetBtn,&QPushButton::clicked,this,&TApptDlg::editPet);

  connect(routineBtn,&QPushButton::clicked,this,&TApptDlg::solveTimeSlots);

  okBtn->setShortcut(QKeySequence(Qt::Key_Enter));
  cancelBtn->setShortcut(QKeySequence(Qt::Key_Cancel));

  connect(okBtn,&QPushButton::clicked,this,&TApptDlg::accept);
  connect(cancelBtn,&QPushButton::clicked,this,&TApptDlg::reject);

  switch (pageTab)
    {
    // этот случай для установки фильтра при экспорте/генерации отчетов, спрятать лишние элементы
    case TMainWnd::UndefinedTab:
      {
      setWindowTitle(tr("Appointment details"));

      dateTimeLbl->hide();
      routineBtn->hide();
      apptDateTimeCombo->hide();

      appendDoctorBtn->hide();
      editDoctorBtn->hide();

      appendClientBtn->hide();
      editClientBtn->hide();

      appendPetBtn->hide();
      editPetBtn->hide();
      break;
      }
    case TMainWnd::ApptsTab:
      {
      doctorCombo->setReadOnly(true);
      appendDoctorBtn->setEnabled(false);
      editDoctorBtn->setEnabled(false);

      routineBtn->setEnabled(false);
      break;
      }
    case TMainWnd::PetsTab:
      {
      petCombo->setReadOnly(true);
      appendPetBtn->setEnabled(false);
      editPetBtn->setEnabled(false);

      clientCombo->setReadOnly(true);
      appendClientBtn->setEnabled(false);
      editClientBtn->setEnabled(false);
      break;
      }
    case TMainWnd::ClientsTab:
      {
      clientCombo->setReadOnly(true);
      appendClientBtn->setEnabled(false);
      editClientBtn->setEnabled(false);
      break;
      }
    case TMainWnd::DoctorsTab:
      {
      doctorCombo->setReadOnly(true);
      appendDoctorBtn->setEnabled(false);
      editDoctorBtn->setEnabled(false);
      break;
      }
    default:
      break;
    }

  actionCombo->setFocus();
  readSettings();
  }

QSqlRecord TApptDlg::record() const
  {
  QSqlRecord record=dataModule()->tableRecord("appts");

  record.setValue("appt_date", apptDateTimeCombo->currentData().toDateTime().date());
  record.setValue("appt_time", apptDateTimeCombo->currentData().toDateTime().time());
  record.setValue("action_id", actionCombo->currentData().toInt());
  record.setValue("doctor_id", doctorCombo->currentData().toInt());
  record.setValue("client_id", clientCombo->currentData().toInt());
  record.setValue("pet_id", petCombo->currentData().toInt());
  record.setValue("state", TApptsModel::Created);

  return record;
  }

void TApptDlg::accept()
  {
  if (m_pageTab==TMainWnd::UndefinedTab)
    {
    TModelDlg::accept();
    return;
    }

  if (apptDateTimeCombo->currentData().toDateTime().date().isValid()==false)
    {
    QMessageBox::critical(this,tr("Error"),tr("It is needed to specify the valid date and time."),QMessageBox::Ok);
    apptDateTimeCombo->setFocus();
    //apptDateEdit->setFocus();
    return;
    }

  if (actionCombo->currentData().toInt()<1)
    {
    QMessageBox::critical(this,tr("Error"),tr("It is needed to specify the action."),QMessageBox::Ok);
    doctorCombo->setFocus();
    return;
    }

  if (doctorCombo->currentData().toInt()<1)
    {
    QMessageBox::critical(this,tr("Error"),tr("It is needed to specify the doctor."),QMessageBox::Ok);
    doctorCombo->setFocus();
    return;
    }

  if (clientCombo->currentData().toInt()<1)
    {
    QMessageBox::critical(this,tr("Error"),tr("It is needed to specify the client."),QMessageBox::Ok);
    clientCombo->setFocus();
    return;
    }

  if (petCombo->currentData().toInt()<1)
    {
    QMessageBox::critical(this,tr("Error"),tr("It is needed to specify the pet."),QMessageBox::Ok);
    petCombo->setFocus();
    return;
    }

  TModelDlg::accept();
  }

void TApptDlg::doctorChanged(int index)
  {
  QMap<QString, QVariant> data=doctorCombo->data(index);
  profileEdit->setText(data.value("profile").toString());

  routineBtn->setEnabled(index>-1);
  editDoctorBtn->setEnabled(doctorCombo->currentIndex()>-1);

  if (actionCombo->currentIndex()==-1)
    actionCombo->setCurrentIndex(actionCombo->findData(data.value("action_id").toUInt()));
  }

void TApptDlg::clientChanged(int index)
  {
  QMap<QString, QVariant> data=clientCombo->data(index);
  phoneEdit->setText(data.value("phone_number").toString());
  telegramEdit->setText(data.value("telegram").toString());

  editClientBtn->setEnabled(clientCombo->currentIndex()>-1);

  TPetsModel *petsModel=qobject_cast<TPetsModel*>(dataModule()->tableModel("pets"));
  // взять, если установлен, сущетсвующий фильтр и добавить принадлежность клиенту
  QStringList filterParts;
  if (petsModel->filter().isEmpty()==false)
    filterParts << petsModel->filter();
  if (clientCombo->currentData().toInt()>0)
    filterParts << QString("pets.client_id=%1").arg(clientCombo->currentData().toString());

  petCombo->setModel(petsModel,{"name","species","breed","uicmm","vet_passport","client_id"},"id",filterParts.join(" and "));
  }

void TApptDlg::petChanged(int index)
  {
  QMap<QString, QVariant> data=petCombo->data(index);
  speciesEdit->setText(data.value("species").toString());
  breedEdit->setText(data.value("breed").toString());

  editPetBtn->setEnabled(petCombo->currentIndex()>-1);

  // если клиент еще не выбран, то ставим клиентом хозяина питомца
  if (clientCombo->currentIndex()==-1)
    clientCombo->setCurrentIndex(clientCombo->findData(data.value("client_id").toInt()));
  }

void TApptDlg::appendDoctor()
  {
  // запомнить текущий индекс в doctorCombo
  qint32 index=doctorCombo->currentIndex();
  // запомнить seq до вызова диалога добавления
  qint32 before=dataModule()->lastAutoInc("doctors");

  TDoctorsModel *doctorsModel=qobject_cast<TDoctorsModel*>(dataModule()->tableModel("doctors"));
  emit updateDataNeeded(doctorsModel, 0);

  // обновить модель в doctorCombo и установить последнюю созданную запись
  doctorCombo->setModel(doctorsModel,{"name","profile","action_id"},"id","doctors.available=true");

  qint32 after=dataModule()->lastAutoInc("doctors");
  if (after!=before)
    index=clientCombo->findData(after);

  if (index>-1)
    doctorCombo->setCurrentIndex(index);
  }

void TApptDlg::editDoctor()
  {
  // запомнить текущий индекс doctorCombo
  quint32 index=doctorCombo->currentIndex();

  TDoctorsModel *doctorsModel=qobject_cast<TDoctorsModel*>(dataModule()->tableModel("doctors"));
  emit updateDataNeeded(doctorsModel, doctorCombo->currentData().toUInt());

  // обновить модель в doctorCombo и установить текущую запись
  doctorCombo->setModel(doctorsModel,{"name","profile","action_id"},"id","doctors.available=true");
  doctorCombo->setCurrentIndex(index);
  }

void TApptDlg::appendClient()
  {
  // запомнить текущие индексы clientCombo и petCombo
  qint32 index=clientCombo->currentIndex();
  qint32 petIndex=petCombo->currentIndex();
  // запомнить seq до вызова диалога добавления
  qint32 before=dataModule()->lastAutoInc("clients");

  TClientsModel *clientsModel=qobject_cast<TClientsModel*>(dataModule()->tableModel("clients"));
  emit updateDataNeeded(clientsModel, 0);

  // обновить модель в clientCombo и установить последнюю созданную запись
  clientCombo->setModel(clientsModel,{"name","phone_number","telegram"},"id");

  qint32 after=dataModule()->lastAutoInc("clients");
  // если реально добавилась запись (seq изменился)
  if (after!=before)
    index=clientCombo->findData(after);

  if (index>-1)
    clientCombo->setCurrentIndex(index);
  // вернуть индекс в petCombo, если клиент не добавился
  if (after==before)
    petCombo->setCurrentIndex(petIndex);
  }

void TApptDlg::editClient()
  {
  // запомнить текущие индексы clientCombo и petCombo
  qint32 index=clientCombo->currentIndex();
  qint32 petIndex=petCombo->currentIndex();

  TClientsModel *clientsModel=qobject_cast<TClientsModel*>(dataModule()->tableModel("clients"));
  emit updateDataNeeded(clientsModel, clientCombo->currentData().toUInt());

  // обновить модель в clientCombo и установить текущую запись
  clientCombo->setModel(clientsModel,{"name","phone_number","telegram"},"id");
  clientCombo->setCurrentIndex(index);

  petCombo->setCurrentIndex(petIndex);
  }

void TApptDlg::appendPet()
  {
  // запомнить текущий индекс в petCombo
  qint32 index=petCombo->currentIndex();
  // запомнить seq до вызова диалога добавления
  qint32 before=dataModule()->lastAutoInc("pets");

  TPetsModel *petsModel=qobject_cast<TPetsModel*>(dataModule()->tableModel("pets"));
  emit updateDataNeeded(petsModel, 0);

  // обновить модель в petCombo и установить последнюю созданную запись
  QString filter;
  if (clientCombo->currentData().toInt()>0)
    filter="pets.client_id="+clientCombo->currentData().toString();

  petCombo->setModel(petsModel,{"name","species","breed","uicmm","vet_passport","client_id"},"id",filter);

  qint32 after=dataModule()->lastAutoInc("pets");
  if (after!=before)
    index=petCombo->findData(after);

  if (index>-1)
    petCombo->setCurrentIndex(index);
  }

void TApptDlg::editPet()
  {
  // запомнить текущиq индекс petCombo
  qint32 index=petCombo->currentIndex();

  TPetsModel *petsModel=qobject_cast<TPetsModel*>(dataModule()->tableModel("pets"));
  emit updateDataNeeded(petsModel, petCombo->currentData().toUInt());

  // обновить модель в petCombo и установить текущую запись
  QString filter;
  if (clientCombo->currentData().toInt()>0)
    filter="pets.client_id="+clientCombo->currentData().toString();

  petCombo->setModel(petsModel,{"name","species","breed","uicmm","vet_passport","client_id"},"id",filter);
  petCombo->setCurrentIndex(index);
  }

void TApptDlg::solveTimeSlots()
  {
  quint32 doctorId=doctorCombo->currentData().toUInt();

  TRoutinesModel *routinesModel=qobject_cast<TRoutinesModel*>(dataModule()->tableModel("routines"));
  routinesModel->setFilter(QString("routines.doctor_id=%1").arg(doctorId));

  TRoutinesFilterModel existRoutineModel(TRoutinesFilterModel::Available);
  existRoutineModel.setSourceModel(routinesModel);

  TApptsModel *apptsModel=qobject_cast<TApptsModel*>(dataModule()->tableModel("appts"));
  apptsModel->setFilter(QString("appts.doctor_id=%1").arg(doctorId));

  TPeriodDlg periodDlg;
  if (periodDlg.exec()!=QDialog::Accepted)
    return;

  // чтобы не делать запросы к БД, просто буду фильтровать уже находящуюся в памяти модель
  TApptsFilterModel apptsFilterModel(this);
  apptsFilterModel.setSourceModel(apptsModel);

  qApp->setOverrideCursor(Qt::WaitCursor);

  QPair<QDate,QDate> period=periodDlg.period();
  apptDateTimeCombo->clear();
  QStringList weekDays={tr("Monday"),tr("Tuesday"),tr("Wednesday"),tr("Thursday"),tr("Friday"),tr("Saturday"),tr("Sunday")};
  for (QDate date=period.first;date<=period.second;date=date.addDays(1))
    {
    quint8 day=((date.weekNumber()+1)%2)*7+(date.dayOfWeek()-1);
    existRoutineModel.setFilterData(day);
    for (quint16 row=0;row<existRoutineModel.rowCount();row++)
      {
      QModelIndex routineIndex=existRoutineModel.mapToSource(existRoutineModel.index(row,routinesModel->fieldIndex("id")));
      QSqlRecord routineRecord=routinesModel->record(routineIndex.row());

      quint16 interval=routineRecord.value("interval").toUInt();

      QDateTime fromAm=QDateTime(date,routineRecord.value("from_am").toTime());
      QDateTime toAm=QDateTime(date,routineRecord.value("to_am").toTime());
      QDateTime fromPm=QDateTime(date,routineRecord.value("from_pm").toTime());
      QDateTime toPm=QDateTime(date,routineRecord.value("to_pm").toTime());

      QDateTime slotTime=fromAm;
      // определяем тайм-слоты, и смотрим назначенные записи, время окончания записи должно укладываться в рабочее время, и не должно быть перехода через полночь
      while (slotTime.addSecs(60*(interval-1))<=toPm) // -1 мин. тут - чтобы зацепить 00:00, если работает до 23:59
        {
        // раньше, чем сейчас нельзя
        if (slotTime<=QDateTime::currentDateTime())
          {
          slotTime=slotTime.addSecs(60*interval);
          continue;
          }

        // перерыв на обед
        if (slotTime.addSecs(60*interval)>toAm && slotTime<fromPm)
          {
          slotTime=fromPm;
          continue;
          }

        // проверить не занят ли уже слот кем-то
        apptsFilterModel.setFilterData(slotTime,slotTime.addSecs(60),{});
        // то, что нужно нам
        if (apptsFilterModel.rowCount()==0) // нет записей на эти дата-время к этому доктору
          {
          QString text=QString("%1 (%2) %3").arg(slotTime.toString("dd.MM.yyyy"),weekDays.at(slotTime.date().dayOfWeek()-1),slotTime.toString("hh:mm"));
          apptDateTimeCombo->addItem(text, slotTime);
          }

        slotTime=slotTime.addSecs(60*interval);
        }
      }
    }

  if (apptDateTimeCombo->count()>0)
    apptDateTimeCombo->showPopup();

  qApp->restoreOverrideCursor();
  }
