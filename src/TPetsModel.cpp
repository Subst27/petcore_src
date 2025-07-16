#include "TPetsModel.h"

TPetsModel::TPetsModel(QObject *parent, const QSqlDatabase &dataBase) : TSqlTableModel(parent,dataBase)
  {
  setTable("pets");
  setTitle(tr("Pets"));

  setHeaderField("name");

  appendRelation(TSqlRelation("client_id","client_name","clients","id","name"));
  appendRelation(TSqlRelation("client_id","passport","clients","id","passport"));
  appendRelation(TSqlRelation("client_id","phone_number","clients","id","phone_number"));

  appendRelation(TSqlRelation("breed_id","breed","breeds","id","breed"));
  appendRelation(TSqlRelation("breed_id","forbidden","breeds","id","forbidden"));

  appendRelation(TSqlRelation("status_id","status","statuses","id","status"));

  setEditStrategy(QSqlTableModel::OnRowChange);
  }

void TPetsModel::setHeaderNames()
  {
  setHeaderData(fieldIndex("id"),Qt::Horizontal,tr("ID"),Qt::DisplayRole);
  setHeaderData(fieldIndex("name"),Qt::Horizontal,tr("Name"),Qt::DisplayRole);
  setHeaderData(fieldIndex("birth_date"),Qt::Horizontal,tr("Birthdate"),Qt::DisplayRole);
  setHeaderData(fieldIndex("uicmm"),Qt::Horizontal,tr("UICMM"),Qt::DisplayRole);
  setHeaderData(fieldIndex("vet_passport"),Qt::Horizontal,tr("Vet passport"),Qt::DisplayRole);

  setHeaderData(fieldIndex("client_id"),Qt::Horizontal,tr("Client ID"),Qt::DisplayRole);
  setHeaderData(fieldIndex("client_name"),Qt::Horizontal,tr("Client name"),Qt::DisplayRole);
  setHeaderData(fieldIndex("phone_number"),Qt::Horizontal,tr("Phone number"),Qt::DisplayRole);
  setHeaderData(fieldIndex("telegram"),Qt::Horizontal,tr("Telegram ID"),Qt::DisplayRole);

  setHeaderData(fieldIndex("breed_id"),Qt::Horizontal,tr("Breed ID"),Qt::DisplayRole);
  setHeaderData(fieldIndex("breed"),Qt::Horizontal,tr("Breed"),Qt::DisplayRole);
  setHeaderData(fieldIndex("forbidden"),Qt::Horizontal,tr("Forbidden"),Qt::DisplayRole);
  setHeaderData(fieldIndex("marking_id"),Qt::Horizontal,tr("Marking ID"),Qt::DisplayRole);
  setHeaderData(fieldIndex("marking"),Qt::Horizontal,tr("Marking"),Qt::DisplayRole);
  setHeaderData(fieldIndex("marking_date"),Qt::Horizontal,tr("Marking date"),Qt::DisplayRole);

  setHeaderData(fieldIndex("status_id"),Qt::Horizontal,tr("Status ID"),Qt::DisplayRole);
  setHeaderData(fieldIndex("status"),Qt::Horizontal,tr("Status"),Qt::DisplayRole);

  setHeaderData(fieldIndex("species_id"),Qt::Horizontal,tr("Species ID"),Qt::DisplayRole);
  setHeaderData(fieldIndex("species"),Qt::Horizontal,tr("Species"),Qt::DisplayRole);
  }

QString TPetsModel::selectStatement() const
  {
  QString statement="select pets.id, pets.name, pets.birth_date, "
                    "pets.marking_id, marking.marking, marking_date, "
                    "pets.uicmm, pets.vet_passport, "
                    "pets.species_id, species.species,"
                    "pets.breed_id, breeds.breed, breeds.forbidden, "
                    "pets.client_id, clients.name as client_name, clients.phone_number, clients.telegram, "
                    "pets.status_id, statuses.status "
                    "from pets "
                    "left join clients on pets.client_id=clients.id "
                    "left join species on pets.species_id=species.id "
                    "left join breeds on pets.breed_id=breeds.id "
                    "left join marking on pets.marking_id=marking.id "
                    "left join statuses on pets.status_id=statuses.id";

  if (filter().isEmpty()==false)
    statement.append(" where "+filter());

  statement.append(" order by pets.name asc");
  return statement;
  }
