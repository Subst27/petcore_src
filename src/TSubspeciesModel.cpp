#include "TSubspeciesModel.h"

TSubspeciesModel::TSubspeciesModel(QObject *parent, const QSqlDatabase &dataBase) : TSqlTableModel(parent, dataBase)
  {
  setTable("subspecies");
  setTitle(tr("Subspecies"));

  setHeaderField("subspecies");

  setEditStrategy(QSqlTableModel::OnFieldChange);
  }

void TSubspeciesModel::setHeaderNames()
  {
  setHeaderData(fieldIndex("id"),Qt::Horizontal,tr("ID"),Qt::DisplayRole);
  setHeaderData(fieldIndex("species_id"),Qt::Horizontal,tr("Species ID"),Qt::DisplayRole);
  setHeaderData(fieldIndex("subspecies"),Qt::Horizontal,tr("Subspecies"),Qt::DisplayRole);
  setHeaderData(fieldIndex("forbidden"),Qt::Horizontal,tr("Forbidden"),Qt::DisplayRole);
  }

QString TSubspeciesModel::selectStatement() const
  {
  QString statement="select subspecies.id, subspecies.species_id, subspecies.subspecies, subspecies.forbidden from subspecies";

  if (filter().isEmpty()==false)
    statement.append(" where "+filter());

  statement.append(" order by subspecies.subspecies asc");
  return statement;
  }
