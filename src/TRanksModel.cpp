#include "TRanksModel.h"

TRanksModel::TRanksModel(QObject *parent, const QSqlDatabase &dataBase) : TSqlTableModel(parent, dataBase)
  {
  setTable("ranks");
  setTitle(tr("Ranks"));

  setHeaderField("rank");

  setEditStrategy(QSqlTableModel::OnFieldChange);
  }

void TRanksModel::setHeaderNames()
  {
  setHeaderData(fieldIndex("id"),Qt::Horizontal,tr("ID"),Qt::DisplayRole);
  setHeaderData(fieldIndex("rank"),Qt::Horizontal,tr("Rank"),Qt::DisplayRole);
  }

QString TRanksModel::selectStatement() const
  {
  QString statement="select ranks.id, ranks.rank from ranks";

  if (filter().isEmpty()==false)
    statement.append(" where "+filter());

  statement.append(" order by ranks.rank asc");
  return statement;
  }
