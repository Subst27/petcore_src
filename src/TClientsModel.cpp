#include "TClientsModel.h"

TClientsModel::TClientsModel(QObject *parent, const QSqlDatabase &dataBase) : TSqlTableModel(parent, dataBase)
  {
  setTable("clients");
  setTitle(tr("Clients"));

  setHeaderField("name");

  setEditStrategy(QSqlTableModel::OnRowChange);
  }

void TClientsModel::setHeaderNames()
  {
  setHeaderData(fieldIndex("id"),Qt::Horizontal,tr("ID"),Qt::DisplayRole);
  setHeaderData(fieldIndex("name"),Qt::Horizontal,tr("Name"),Qt::DisplayRole);
  setHeaderData(fieldIndex("phone_number"),Qt::Horizontal,tr("Phone number"),Qt::DisplayRole);
  setHeaderData(fieldIndex("passport"),Qt::Horizontal,tr("Passport"),Qt::DisplayRole);
  setHeaderData(fieldIndex("telegram"),Qt::Horizontal,tr("Telegram ID"),Qt::DisplayRole);
  setHeaderData(fieldIndex("address"),Qt::Horizontal,tr("Address"),Qt::DisplayRole);
  }

QString TClientsModel::selectStatement() const
  {
  QString statement="select clients.id, clients.name, clients.phone_number, clients.telegram, clients.passport, clients.address from clients";

  if (filter().isEmpty()==false)
    statement.append(" where "+filter());

  statement.append(" order by clients.name asc");//+orderByClause());
  return statement;
  }
