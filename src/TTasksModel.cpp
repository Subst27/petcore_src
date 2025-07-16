#include "TTasksModel.h"

TTasksModel::TTasksModel(QObject *parent, const QSqlDatabase &dataBase) : TSqlTableModel(parent,dataBase)
  {
  setTable("tasks");
  setTitle(tr("Tasks"));

  setHeaderField("task_date");

  setEditStrategy(QSqlTableModel::OnRowChange);
  }

void TTasksModel::setHeaderNames()
  {
  setHeaderData(fieldIndex("id"),Qt::Horizontal,tr("ID"),Qt::DisplayRole);
  setHeaderData(fieldIndex("task_date"),Qt::Horizontal,tr("Date"),Qt::DisplayRole);
  setHeaderData(fieldIndex("task"),Qt::Horizontal,tr("Task"),Qt::DisplayRole);
  setHeaderData(fieldIndex("completed"),Qt::Horizontal,tr("Completed"),Qt::DisplayRole);
  }

QString TTasksModel::selectStatement() const
  {
  QString statement="select tasks.id, tasks.task_date,  "
                    "tasks.task, tasks.completed "
                    "from tasks";

  if (filter().isEmpty()==false)
    statement.append(" where "+filter());

  statement.append(" order by tasks.completed, tasks.task_date asc");
  return statement;
  }
