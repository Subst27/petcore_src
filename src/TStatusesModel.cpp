#include "TStatusesModel.h"

TStatusesModel::TStatusesModel(QObject *parent, const QSqlDatabase &dataBase) : TSqlTableModel(parent, dataBase)
  {
  setTable("subjects");
  setTitle(tr("Statuses"));

  setHeaderField("status");

  setEditStrategy(QSqlTableModel::OnFieldChange);
  }

void TStatusesModel::setHeaderNames()
  {
  setHeaderData(fieldIndex("id"),Qt::Horizontal,tr("ID"),Qt::DisplayRole);
  setHeaderData(fieldIndex("status"),Qt::Horizontal,tr("Status"),Qt::DisplayRole);
  }

QString TStatusesModel::selectStatement() const
  {
  QString statement="select statuses.id, statuses.status from statuses";

  if (filter().isEmpty()==false)
    statement.append(" where "+filter());

  statement.append(" order by statuses.status asc");
  return statement;
  }
