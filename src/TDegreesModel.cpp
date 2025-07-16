#include "TDegreesModel.h"

TDegreesModel::TDegreesModel(QObject *parent, const QSqlDatabase &dataBase) : TSqlTableModel(parent, dataBase)
  {
  setTable("degrees");
  setTitle(tr("Degrees"));

  setHeaderField("degree");

  setEditStrategy(QSqlTableModel::OnFieldChange);
  }

void TDegreesModel::setHeaderNames()
  {
  setHeaderData(fieldIndex("id"),Qt::Horizontal,tr("ID"),Qt::DisplayRole);
  setHeaderData(fieldIndex("degree"),Qt::Horizontal,tr("Degree"),Qt::DisplayRole);
  }

QString TDegreesModel::selectStatement() const
  {
  QString statement="select degrees.id, degrees.degree from degrees";

  if (filter().isEmpty()==false)
    statement.append(" where "+filter());

  statement.append(" order by degrees.degree asc");//+orderByClause());
  return statement;
  }
