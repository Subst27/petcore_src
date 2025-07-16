#include "TRemindersModel.h"

TRemindersModel::TRemindersModel(QObject *parent, const QSqlDatabase &dataBase) : TSqlTableModel(parent,dataBase)
  {
  setTable("reminders");
  setTitle(tr("Reminders"));

  setHeaderField("client_name");

  appendRelation(TSqlRelation("client_id","client_name","clients","id","name"));

  setEditStrategy(QSqlTableModel::OnRowChange);
  }

void TRemindersModel::setHeaderNames()
  {
  setHeaderData(fieldIndex("id"),Qt::Horizontal,tr("ID"),Qt::DisplayRole);
  setHeaderData(fieldIndex("reminder_date"),Qt::Horizontal,tr("Date"),Qt::DisplayRole);
  setHeaderData(fieldIndex("client_id"),Qt::Horizontal,tr("Client ID"),Qt::DisplayRole);
  setHeaderData(fieldIndex("client_name"),Qt::Horizontal,tr("Client name"),Qt::DisplayRole);
  setHeaderData(fieldIndex("reminder"),Qt::Horizontal,tr("Reminder"),Qt::DisplayRole);
  }

QString TRemindersModel::selectStatement() const
  {
  QString statement="select reminders.id, reminders.reminder_date,  "
                    "reminders.client_id, clients.name as client_name, "
                    "reminders.reminder "
                    "from reminders "
                    "left join clients on reminders.client_id=clients.id";

  if (filter().isEmpty()==false)
    statement.append(" where "+filter());

  statement.append(" order by reminders.reminder_date asc");//+orderByClause());
  return statement;
  }
