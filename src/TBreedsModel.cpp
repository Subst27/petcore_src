#include "TBreedsModel.h"

TBreedsModel::TBreedsModel(QObject *parent, const QSqlDatabase &dataBase) : TSqlTableModel(parent, dataBase)
  {
  setTable("breeds");
  setTitle(tr("Breeds"));

  setHeaderField("breed");

  appendRelation(TSqlRelation("species_id","species","species","id","species"));
  appendRelation(TSqlRelation("subspecies_id","subspecies","subspecies","id","subspecies"));

  setEditStrategy(QSqlTableModel::OnFieldChange);
  }

void TBreedsModel::setHeaderNames()
  {
  setHeaderData(fieldIndex("id"),Qt::Horizontal,tr("ID"),Qt::DisplayRole);
  setHeaderData(fieldIndex("species_id"),Qt::Horizontal,tr("Species ID"),Qt::DisplayRole);
  setHeaderData(fieldIndex("species"),Qt::Horizontal,tr("Species"),Qt::DisplayRole);
  setHeaderData(fieldIndex("subspecies_id"),Qt::Horizontal,tr("Subspecies ID"),Qt::DisplayRole);
  setHeaderData(fieldIndex("subspecies"),Qt::Horizontal,tr("Subspecies"),Qt::DisplayRole);
  setHeaderData(fieldIndex("breed"),Qt::Horizontal,tr("Breed"),Qt::DisplayRole);
  setHeaderData(fieldIndex("forbidden"),Qt::Horizontal,tr("Forbidden"),Qt::DisplayRole);
  }

QString TBreedsModel::selectStatement() const
  {
  QString statement="select breeds.id, "
                    "breeds.species_id, species.species, "
                    "breeds.subspecies_id, subspecies.subspecies, "
                    "breeds.breed, breeds.forbidden "
                    "from breeds "
                    "left join species on breeds.species_id=species.id "
                    "left join subspecies on breeds.subspecies_id=subspecies.id";

  if (filter().isEmpty()==false)
    statement.append(" where "+filter());

  statement.append(" order by breeds.breed asc");
  return statement;
  }
