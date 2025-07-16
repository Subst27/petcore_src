#include "TDataModule.h"

#include <QDebug>

#include <QSqlQuery>
#include <QSqlField>

#include "TSqlTableModel.h"

/* справочники */
#include "TStatusesModel.h"
#include "TMarkingModel.h"
#include "TProfilesModel.h"

#include "TDegreesModel.h"
#include "TRanksModel.h"
#include "TPetagesModel.h"
#include "TActionsModel.h"

#include "TSpeciesModel.h"
#include "TSubspeciesModel.h"
#include "TBreedsModel.h"

/* рабочие данные */
#include "TDoctorsModel.h"
#include "TClientsModel.h"
#include "TPetsModel.h"

#include "TRemindersModel.h"
#include "TRoutinesModel.h"
#include "TApptsModel.h"

TDataModule *TDataModule::m_instance=nullptr;
// TODO: присобачить Q_GLOBAL_STATIC ?
TDataModule::TDataModule(QObject *parent) : QObject(parent)
  {
  m_instance=this; // static variable

  m_statusesModel=nullptr;
  m_markingModel=nullptr;
  m_profilesModel=nullptr;

  m_degreesModel=nullptr;
  m_ranksModel=nullptr;
  m_petagesModel=nullptr;
  m_actionsModel=nullptr;

  m_speciesModel=nullptr;
  m_subspeciesModel=nullptr;
  m_breedsModel=nullptr;

  m_doctorsModel=nullptr;
  m_clientsModel=nullptr;
  m_petsModel=nullptr;

  m_remindersModel=nullptr;
  m_routinesModel=nullptr;
  m_apptsModel=nullptr;
  }

TDataModule *TDataModule::instance()
  {
  return m_instance;
  }

bool TDataModule::openBase(const QString &baseName)
  {
  if (m_petcoreBase.isOpen()==true)
    m_petcoreBase.close();

  if (baseName.isEmpty()==true && QSqlDatabase::contains("petcore_base")==false)
    return false;

  if (QSqlDatabase::contains("petcore_base")==false)
    m_petcoreBase=QSqlDatabase::addDatabase("QSQLITE","petcore_base");

  m_petcoreBase.setDatabaseName(baseName);
  if (m_petcoreBase.open()==false || m_petcoreBase.isValid()==false || m_petcoreBase.tables().contains("pets")==false)
    {
    if (m_petcoreBase.isOpen()==true)
      m_petcoreBase.close();

    return false;
    }

  correctStructure(); // в будущем нужно будет, пока просто, чтоб не забыть

  // создать модели, если еще не созданы (могут быть уже созданы, если переоткрываем БД) и добавить их в хэш моделей
  if (m_statusesModel==nullptr)
    m_statusesModel=new TStatusesModel(this,m_petcoreBase);
  m_tableHash["statuses"]=m_statusesModel;

  if (m_markingModel==nullptr)
    m_markingModel=new TMarkingModel(this,m_petcoreBase);
  m_tableHash["marking"]=m_markingModel;

  if (m_profilesModel==nullptr)
    m_profilesModel=new TProfilesModel(this,m_petcoreBase);
  m_tableHash["profiles"]=m_profilesModel;

  if (m_degreesModel==nullptr)
    m_degreesModel=new TDegreesModel(this,m_petcoreBase);
  m_tableHash["degrees"]=m_degreesModel;

  if (m_ranksModel==nullptr)
    m_ranksModel=new TRanksModel(this,m_petcoreBase);
  m_tableHash["ranks"]=m_ranksModel;

  if (m_petagesModel==nullptr)
    m_petagesModel=new TPetagesModel(this,m_petcoreBase);
  m_tableHash["petages"]=m_petagesModel;

  if (m_actionsModel==nullptr)
    m_actionsModel=new TActionsModel(this,m_petcoreBase);
  m_tableHash["actions"]=m_actionsModel;

  if (m_speciesModel==nullptr)
    m_speciesModel=new TSpeciesModel(this,m_petcoreBase);
  m_tableHash["species"]=m_speciesModel;

  if (m_subspeciesModel==nullptr)
    m_subspeciesModel=new TSubspeciesModel(this,m_petcoreBase);
  m_tableHash["subspecies"]=m_subspeciesModel;

  if (m_breedsModel==nullptr)
    m_breedsModel=new TBreedsModel(this,m_petcoreBase);
  m_tableHash["breeds"]=m_breedsModel;

  if (m_doctorsModel==nullptr)
    m_doctorsModel=new TDoctorsModel(this,m_petcoreBase);
  m_tableHash["doctors"]=m_doctorsModel;

  if (m_clientsModel==nullptr)
    m_clientsModel=new TClientsModel(this,m_petcoreBase);
  m_tableHash["clients"]=m_clientsModel;

  if (m_petsModel==nullptr)
    m_petsModel=new TPetsModel(this,m_petcoreBase);
  m_tableHash["pets"]=m_petsModel;

  if (m_remindersModel==nullptr)
    m_remindersModel=new TRemindersModel(this,m_petcoreBase);
  m_tableHash["reminders"]=m_remindersModel;

  if (m_routinesModel==nullptr)
    m_routinesModel=new TRoutinesModel(this,m_petcoreBase);
  m_tableHash["routines"]=m_routinesModel;

  if (m_apptsModel==nullptr)
    m_apptsModel=new TApptsModel(this,m_petcoreBase);
  m_tableHash["appts"]=m_apptsModel;

  // обзовем все хедеры у всех моделей
  foreach (TSqlTableModel *model,m_tableHash.values())
    {
    updateModel(model);
    model->setHeaderNames();
    }

  //m_petcoreBase.rollback();
  return true;
  }

void TDataModule::closeBase()
  {
  m_petcoreBase.commit();

  if (m_petcoreBase.isOpen()==true)
    m_petcoreBase.close();
  }

QSqlDatabase TDataModule::database() const
  {
  return m_petcoreBase;
  }

QString TDataModule::baseName() const
  {
  return m_petcoreBase.databaseName();
  }

TSqlTableModel *TDataModule::tableModel(const QString &table)
  {
  Q_ASSERT_X(m_tableHash.contains(table)==true,"TDataModule::tableModel",QString("Table doesn't exists: '%1'").arg(table).toLocal8Bit().data());
  return m_tableHash.value(table);
  }

QSqlRecord TDataModule::tableRecord(const QString &table) const
  {
  QSqlRecord record=m_petcoreBase.record(table);
  for (qint16 i=record.count()-1;i>-1;i--)
    {
    if (record.field(i).isAutoValue()==true)
      record.remove(i);
    }

  return record;
  }

qint8 TDataModule::fieldIndex(const QString &table, const QString &field)
  {
  TSqlTableModel *model=tableModel(table);
  if (model==nullptr)
    return -1;

  QSqlRecord record=tableRecord(table);
  if (field=="header_field")
    return record.indexOf(model->headerField());

  return record.indexOf(field);
  }

qint32 TDataModule::lastAutoInc(const QString &tableName)
  {
  if (m_tableHash.contains(tableName)==false)
    return -1;

  // NOTE: проверить ещ наличие автоинкрементного поля?
  QSqlQuery query(m_petcoreBase);
  query.prepare(QString("select seq from sqlite_sequence where name='%1'").arg(tableName));
  if (query.exec()==false || query.first()==false)
    return -1;

  return query.value("seq").toUInt();
  }

void TDataModule::updateModel(TSqlTableModel *model)
  {
  if (model==nullptr)
    return;

  model->select();
  while (model->canFetchMore())
    model->fetchMore();
  }

void TDataModule::updateModel(const QString &table)
  {
  TSqlTableModel *model=tableModel(table);
  updateModel(model);
  }

void TDataModule::correctStructure()
  {

  }

