#include "TPetagesModel.h"

TPetagesModel::TPetagesModel(QObject *parent, const QSqlDatabase &dataBase) : TSqlTableModel(parent, dataBase)
  {
  setTable("petages");
  setTitle(tr("Pet ages"));

  setHeaderField("pet_age");

  setEditStrategy(QSqlTableModel::OnFieldChange);
  }

void TPetagesModel::setHeaderNames()
  {
  setHeaderData(fieldIndex("id"),Qt::Horizontal,tr("ID"),Qt::DisplayRole);
  setHeaderData(fieldIndex("pet_age"),Qt::Horizontal,tr("Pet age"),Qt::DisplayRole);
  }

QString TPetagesModel::selectStatement() const
  {
  QString statement="select petages.id, petages.pet_age from petages";

  if (filter().isEmpty()==false)
    statement.append(" where "+filter());

  statement.append(" order by petages.pet_age asc");
  return statement;
  }
