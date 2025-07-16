#include "TActionsModel.h"

TActionsModel::TActionsModel(QObject *parent, const QSqlDatabase &dataBase) : TSqlTableModel(parent, dataBase)
  {
  setTable("actions");
  setTitle(tr("Actions"));

  setHeaderField("action");

  setEditStrategy(QSqlTableModel::OnFieldChange);
  }

void TActionsModel::setHeaderNames()
  {
  setHeaderData(fieldIndex("id"),Qt::Horizontal,tr("ID"),Qt::DisplayRole);
  setHeaderData(fieldIndex("action"),Qt::Horizontal,tr("Action"),Qt::DisplayRole);
  }

QString TActionsModel::selectStatement() const
  {
  QString statement="select actions.id, actions.action from actions";

  if (filter().isEmpty()==false)
    statement.append(" where "+filter());

  QString order=orderByClause().isEmpty()==true ? " order by actions.action asc" : orderByClause();
  statement.append(order);
  return statement;
  }
