#include "TApptsModel.h"

TApptsModel::TApptsModel(QObject *parent, const QSqlDatabase &dataBase) : TSqlTableModel(parent,dataBase)
  {
  setTable("appts");
  setTitle(tr("Appointments"));

  setHeaderField("appt_date");

  appendRelation(TSqlRelation("doctor_id","doctor_name","doctors","id","name"));
  appendRelation(TSqlRelation("doctor_id","interval","doctors","id","interval"));
  appendRelation(TSqlRelation("doctor_id","profile_id","doctors","id","profile_id"));

  appendRelation(TSqlRelation("client_id","client_name","clients","id","name"));
  appendRelation(TSqlRelation("client_id","phone_number","clients","id","phone_number"));
  appendRelation(TSqlRelation("client_id","telegram","clients","id","telegram"));

  appendRelation(TSqlRelation("pet_id","pet_name","pets","id","name"));

  appendRelation(TSqlRelation("profile_id","profile","profiles","id","profile"));

  appendRelation(TSqlRelation("species_id","species","species","id","species"));
  appendRelation(TSqlRelation("breed_id","breed","breeds","id","breed"));

  appendRelation(TSqlRelation("actions_id","actioon","actions","id","action"));

  setEditStrategy(QSqlTableModel::OnRowChange);
  }

void TApptsModel::setHeaderNames()
  {
  setHeaderData(fieldIndex("id"),Qt::Horizontal,tr("ID"),Qt::DisplayRole);
  setHeaderData(fieldIndex("appt_date"),Qt::Horizontal,tr("Appt date"),Qt::DisplayRole);
  setHeaderData(fieldIndex("appt_time"),Qt::Horizontal,tr("Appt time"),Qt::DisplayRole);

  setHeaderData(fieldIndex("doctor_id"),Qt::Horizontal,tr("Doctor ID"),Qt::DisplayRole);
  setHeaderData(fieldIndex("doctor_name"),Qt::Horizontal,tr("Doctor name"),Qt::DisplayRole);
  /*setHeaderData(fieldIndex("doctor_phone"),Qt::Horizontal,tr("Doctor phone"),Qt::DisplayRole);
  setHeaderData(fieldIndex("doctor_telegram"),Qt::Horizontal,tr("Doctor telegram ID"),Qt::DisplayRole);*/

  setHeaderData(fieldIndex("interval"),Qt::Horizontal,tr("Interval"),Qt::DisplayRole);
  setHeaderData(fieldIndex("available"),Qt::Horizontal,tr("Available"),Qt::DisplayRole);

  setHeaderData(fieldIndex("profile_id"),Qt::Horizontal,tr("Profile ID"),Qt::DisplayRole);
  setHeaderData(fieldIndex("profile"),Qt::Horizontal,tr("Profile"),Qt::DisplayRole);

  setHeaderData(fieldIndex("client_id"),Qt::Horizontal,tr("Client ID"),Qt::DisplayRole);
  setHeaderData(fieldIndex("client_name"),Qt::Horizontal,tr("Client name"),Qt::DisplayRole);
  setHeaderData(fieldIndex("phone_number"),Qt::Horizontal,tr("Phone number"),Qt::DisplayRole);
  setHeaderData(fieldIndex("telegram"),Qt::Horizontal,tr("Telegram ID"),Qt::DisplayRole);

  setHeaderData(fieldIndex("pet_id"),Qt::Horizontal,tr("Pet ID"),Qt::DisplayRole);
  setHeaderData(fieldIndex("pet_name"),Qt::Horizontal,tr("Pet name"),Qt::DisplayRole);

  setHeaderData(fieldIndex("species_id"),Qt::Horizontal,tr("Species ID"),Qt::DisplayRole);
  setHeaderData(fieldIndex("species"),Qt::Horizontal,tr("Species"),Qt::DisplayRole);

  setHeaderData(fieldIndex("breed_id"),Qt::Horizontal,tr("Breed ID"),Qt::DisplayRole);
  setHeaderData(fieldIndex("breed"),Qt::Horizontal,tr("Breed"),Qt::DisplayRole);

  setHeaderData(fieldIndex("action_id"),Qt::Horizontal,tr("Action ID"),Qt::DisplayRole);
  setHeaderData(fieldIndex("action"),Qt::Horizontal,tr("Action"),Qt::DisplayRole);

  setHeaderData(fieldIndex("state"),Qt::Horizontal,tr("State"),Qt::DisplayRole);
  }

QString TApptsModel::textByState(States state)
  {
  switch (state)
    {
    case TApptsModel::Created:
     return QString();

    case TApptsModel::Started:
      return tr("started");

    case TApptsModel::Finished:
      return tr("finished");
    }

  return QString();
  }

// WARNING: в aptts и TApptsModel date_time instead app_date + app_time
QString TApptsModel::selectStatement() const
  {
  QString statement="select appts.id, appts.appt_date, appts.appt_time, appts.state, "
                    "appts.action_id, actions.action, "
                    "appts.doctor_id, doctors.name as doctor_name, doctors.interval, doctors.available, "
                    "doctors.profile_id, profiles.profile, "
                    "appts.client_id, clients.name as client_name, clients.phone_number, clients.telegram, "
                    "appts.pet_id, pets.name as pet_name, "
                    "pets.species_id, species.species, "
                    "pets.breed_id, breeds.breed "
                    "from appts "
                    "left join actions on appts.action_id=actions.id "
                    "left join doctors on appts.doctor_id=doctors.id "
                    "left join profiles on doctors.profile_id=profiles.id "
                    "left join clients on appts.client_id=clients.id "
                    "left join pets on appts.pet_id=pets.id "
                    "left join species on pets.species_id=species.id "
                    "left join breeds on pets.breed_id=breeds.id";

  if (filter().isEmpty()==false)
    statement.append(" where "+filter());

  statement.append(" order by appts.appt_date, appts.appt_time, doctors.name asc");
  return statement;
  }
