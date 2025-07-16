#include "TSpeciesModel.h"

TSpeciesModel::TSpeciesModel(QObject *parent, const QSqlDatabase &dataBase) : TSqlTableModel(parent, dataBase)
  {
  setTable("species");
  setTitle(tr("Species"));

  setHeaderField("species");

  setEditStrategy(QSqlTableModel::OnFieldChange);
  }

void TSpeciesModel::setHeaderNames()
  {
  setHeaderData(fieldIndex("id"),Qt::Horizontal,tr("ID"),Qt::DisplayRole);
  setHeaderData(fieldIndex("species"),Qt::Horizontal,tr("Species"),Qt::DisplayRole);
  setHeaderData(fieldIndex("forbidden"),Qt::Horizontal,tr("Forbidden"),Qt::DisplayRole);
  }

QString TSpeciesModel::selectStatement() const
  {
  QString statement="select species.id, species.species, species.forbidden from species";

  if (filter().isEmpty()==false)
    statement.append(" where "+filter());

  statement.append(" order by species.species asc");
  return statement;
  }
