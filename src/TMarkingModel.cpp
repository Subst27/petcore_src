#include "TMarkingModel.h"

TMarkingModel::TMarkingModel(QObject *parent, const QSqlDatabase &dataBase) : TSqlTableModel(parent, dataBase)
  {
  setTable("marking");
  setTitle(tr("Marking"));

  setHeaderField("marking");

  setEditStrategy(QSqlTableModel::OnFieldChange);
  }

void TMarkingModel::setHeaderNames()
  {
  setHeaderData(fieldIndex("id"),Qt::Horizontal,tr("ID"),Qt::DisplayRole);
  setHeaderData(fieldIndex("marking"),Qt::Horizontal,tr("Marking"),Qt::DisplayRole);
  }

QString TMarkingModel::selectStatement() const
  {
  QString statement="select marking.id, marking.marking from marking";

  if (filter().isEmpty()==false)
    statement.append(" where "+filter());

  statement.append(" order by marking.marking asc");
  return statement;
  }
