#include "TDoctorsModel.h"

TDoctorsModel::TDoctorsModel(QObject *parent, const QSqlDatabase &dataBase) : TSqlTableModel(parent, dataBase)
  {
  setTable("doctors");
  setTitle(tr("Doctors"));

  setHeaderField("name");

  appendRelation(TSqlRelation("profile_id","profile","profiles","id","profile"));

  appendRelation(TSqlRelation("degree_id","degree","degrees","id","degree"));
  appendRelation(TSqlRelation("rank_id","rank","ranks","id","rank"));

  appendRelation(TSqlRelation("petage_id","pet_age","petages","id","pet_age"));
  appendRelation(TSqlRelation("action_id","action","actions","id","action"));

  setEditStrategy(QSqlTableModel::OnRowChange);
  }

void TDoctorsModel::setHeaderNames()
  {
  setHeaderData(fieldIndex("id"),Qt::Horizontal,tr("ID"),Qt::DisplayRole);
  setHeaderData(fieldIndex("name"),Qt::Horizontal,tr("Name"),Qt::DisplayRole);
  setHeaderData(fieldIndex("birth_date"),Qt::Horizontal,tr("Birthdate"),Qt::DisplayRole);
  setHeaderData(fieldIndex("gender"),Qt::Horizontal,tr("Gender"),Qt::DisplayRole);
  setHeaderData(fieldIndex("phone_number"),Qt::Horizontal,tr("Phone number"),Qt::DisplayRole);
  setHeaderData(fieldIndex("telegram"),Qt::Horizontal,tr("Telegram ID"),Qt::DisplayRole);
  setHeaderData(fieldIndex("email"),Qt::Horizontal,tr("Email"),Qt::DisplayRole);
  setHeaderData(fieldIndex("address"),Qt::Horizontal,tr("Address"),Qt::DisplayRole);
  setHeaderData(fieldIndex("certificate"),Qt::Horizontal,tr("Certificate"),Qt::DisplayRole);
  setHeaderData(fieldIndex("certificate_date"),Qt::Horizontal,tr("Certificate date"),Qt::DisplayRole);

  setHeaderData(fieldIndex("profile_id"),Qt::Horizontal,tr("Profile ID"),Qt::DisplayRole);
  setHeaderData(fieldIndex("profile"),Qt::Horizontal,tr("Profile"),Qt::DisplayRole);

  setHeaderData(fieldIndex("experience"),Qt::Horizontal,tr("Experience"),Qt::DisplayRole);

  setHeaderData(fieldIndex("degree_id"),Qt::Horizontal,tr("Degree ID"),Qt::DisplayRole);
  setHeaderData(fieldIndex("degree"),Qt::Horizontal,tr("Degree"),Qt::DisplayRole);

  setHeaderData(fieldIndex("rank_id"),Qt::Horizontal,tr("Rank ID"),Qt::DisplayRole);
  setHeaderData(fieldIndex("rank"),Qt::Horizontal,tr("Rank"),Qt::DisplayRole);

  setHeaderData(fieldIndex("petage_id"),Qt::Horizontal,tr("Pet age ID"),Qt::DisplayRole);
  setHeaderData(fieldIndex("pet_age"),Qt::Horizontal,tr("Pet age"),Qt::DisplayRole);

  setHeaderData(fieldIndex("action_id"),Qt::Horizontal,tr("Action ID"),Qt::DisplayRole);
  setHeaderData(fieldIndex("action"),Qt::Horizontal,tr("Action"),Qt::DisplayRole);

  setHeaderData(fieldIndex("interval"),Qt::Horizontal,tr("Interval"),Qt::DisplayRole);

  setHeaderData(fieldIndex("preffered"),Qt::Horizontal,tr("Preffered"),Qt::DisplayRole);
  setHeaderData(fieldIndex("available"),Qt::Horizontal,tr("Available"),Qt::DisplayRole);
  }

QString TDoctorsModel::selectStatement() const
  {
  QString statement="select doctors.id, doctors.name, "
                    "doctors.profile_id, profiles.profile, "
                    "doctors.birth_date, doctors.gender, "
                    "doctors.phone_number, doctors.email, doctors.telegram, doctors.address, "
                    "doctors.certificate, doctors.certificate_date, "
                    "doctors.experience, "
                    "doctors.degree_id, degrees.degree, "
                    "doctors.rank_id, ranks.rank, "
                    "doctors.petage_id, petages.pet_age, "
                    "doctors.action_id, actions.action, "
                    "doctors.interval, doctors.preffered, doctors.available "
                    "from doctors "
                    "left join profiles on doctors.profile_id=profiles.id "
                    "left join degrees on doctors.degree_id=degrees.id "
                    "left join ranks on doctors.rank_id=ranks.id "
                    "left join petages on doctors.petage_id=petages.id "
                    "left join actions on doctors.action_id=actions.id";

  if (filter().isEmpty()==false)
    statement.append(" where "+filter());

  statement.append(" order by doctors.name asc");//+orderByClause());
  return statement;
  }
