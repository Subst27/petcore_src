#include "TProfilesModel.h"

TProfilesModel::TProfilesModel(QObject *parent, const QSqlDatabase &dataBase) : TSqlTableModel(parent, dataBase)
  {
  setTable("profiles");
  setTitle(tr("Profiles"));

  setHeaderField("profile");

  setEditStrategy(QSqlTableModel::OnFieldChange);
  }

void TProfilesModel::setHeaderNames()
  {
  setHeaderData(fieldIndex("id"),Qt::Horizontal,tr("ID"),Qt::DisplayRole);
  setHeaderData(fieldIndex("profile"),Qt::Horizontal,tr("Profile"),Qt::DisplayRole);
  }

QString TProfilesModel::selectStatement() const
  {
  QString statement="select profiles.id, profiles.profile from profiles";

  if (filter().isEmpty()==false)
    statement.append(" where "+filter());

  statement.append(" order by profiles.profile asc");//+orderByClause());
  return statement;
  }
