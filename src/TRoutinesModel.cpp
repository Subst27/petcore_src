#include "TRoutinesModel.h"

TRoutinesModel::TRoutinesModel(QObject *parent, const QSqlDatabase &dataBase) : TSqlTableModel(parent,dataBase)
  {
  setTable("routines");
  setTitle(tr("Routines"));

  setHeaderField("day");

  appendRelation(TSqlRelation("doctor_id","name","docotrs","id","name"));
  appendRelation(TSqlRelation("doctor_id","phone_number","docotrs","id","phone_number"));
  appendRelation(TSqlRelation("doctor_id","telegram","docotrs","id","telegram"));
  appendRelation(TSqlRelation("doctor_id","interval","docotrs","id","interval"));
  appendRelation(TSqlRelation("doctor_id","available","docotrs","id","available"));

  appendRelation(TSqlRelation("profile_id","profile","profiles","id","profile"));

  setEditStrategy(QSqlTableModel::OnManualSubmit);
  }

void TRoutinesModel::setHeaderNames()
  {
  setHeaderData(fieldIndex("id"),Qt::Horizontal,tr("ID"),Qt::DisplayRole);
  setHeaderData(fieldIndex("day"),Qt::Horizontal,tr("Day"),Qt::DisplayRole);
  setHeaderData(fieldIndex("doctor_id"),Qt::Horizontal,tr("Doctor ID"),Qt::DisplayRole);
  setHeaderData(fieldIndex("name"),Qt::Horizontal,tr("Name"),Qt::DisplayRole);
  setHeaderData(fieldIndex("phone_number"),Qt::Horizontal,tr("Phone number"),Qt::DisplayRole);
  setHeaderData(fieldIndex("telegram"),Qt::Horizontal,tr("Telegram ID"),Qt::DisplayRole);

  setHeaderData(fieldIndex("interval"),Qt::Horizontal,tr("Interval"),Qt::DisplayRole);
  setHeaderData(fieldIndex("available"),Qt::Horizontal,tr("Available"),Qt::DisplayRole);

  setHeaderData(fieldIndex("profile_id"),Qt::Horizontal,tr("Profile ID"),Qt::DisplayRole);
  setHeaderData(fieldIndex("profile"),Qt::Horizontal,tr("Profile"),Qt::DisplayRole);

  setHeaderData(fieldIndex("from_am"),Qt::Horizontal,tr("AM from"),Qt::DisplayRole);
  setHeaderData(fieldIndex("to_am"),Qt::Horizontal,tr("AM to"),Qt::DisplayRole);
  setHeaderData(fieldIndex("from_pm"),Qt::Horizontal,tr("PM from"),Qt::DisplayRole);
  setHeaderData(fieldIndex("to_pm"),Qt::Horizontal,tr("PM to"),Qt::DisplayRole);
  }

QString TRoutinesModel::selectStatement() const
  {
  QString statement="select routines.id, routines.day, routines.from_am, routines.to_am, routines.from_pm, routines.to_pm, "
                    "routines.doctor_id, doctors.name, doctors.phone_number, doctors.telegram, doctors.interval, doctors.available, "
                    "doctors.profile_id, profiles.profile "
                    "from routines "
                    "left join doctors on routines.doctor_id=doctors.id "
                    "left join profiles on doctors.profile_id=profiles.id";

  if (filter().isEmpty()==false)
    statement.append(" where "+filter());

  statement.append(" order by doctors.name, routines.day asc");
  return statement;
  }
