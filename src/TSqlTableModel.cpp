#include "TSqlTableModel.h"

#include <QDebug>

#include <QSqlRecord>

TSqlTableModel::TSqlTableModel(QObject *parent, const QSqlDatabase &dataBase) : QSqlTableModel(parent, dataBase), m_relations(QList <TSqlRelation>())
  {
  // NOTE: мне, похоже, не нужны TSqlRelation в этом проекте, overhead получился. Порефакторить - убрать?
  }

bool TSqlTableModel::select()
  {
  bool success=QSqlTableModel::select();
  while (canFetchMore()==true)
    fetchMore();

  return success;
  }

bool TSqlTableModel::clearModel()
  {
  quint32 size=rowCount();
  bool success=true;
  for (qint32 i=size-1;i>-1;i--)
    success &= removeRow(i);

  return success;
  }

/*bool TSqlTableModel::removeRows(int row, int count, const QModelIndex &parent)
  {
  beginRemoveRows(parent,row,row+count);
  for (quint32 i=row+count;i>row-1;row--)

  }*/

/*bool TSqlTableModel::removeRow(int row, const QModelIndex &parent)
  {
  beginRemoveRows(parent,row,1);

  QSqlQuery query(database());
  query.prepare(QString("delete from '%1' where id=%2").arg(tableName(),index(row,fieldIndex("id")).data().toString()));
  bool success=query.exec();
  endRemoveRows();

  return success;
  }*/

void TSqlTableModel::setTitle(const QString &title)
  {
  if (m_title!=title)
    {
    m_title=title;
    emit titleChanged(m_title);
    }
  }

QString TSqlTableModel::title() const
  {
  return m_title;
  }

void TSqlTableModel::setHeaderField(const QString &field)
  {
  if (record().contains(field) && m_headerField!=field)
    {
    m_headerField=field;
    emit headerFieldChanged(m_headerField);
    }
  }

QString TSqlTableModel::headerField() const
  {
  return m_headerField;
  }

void TSqlTableModel::setRelations(const QList<TSqlRelation> &relations)
  {
  bool valid=true;
  foreach (const TSqlRelation &relation,relations)
    valid &= isValidRelation(relation);

  if (valid)
    m_relations=relations;
  }

void TSqlTableModel::clearRelations()
  {
  m_relations.clear();
  }

void TSqlTableModel::appendRelation(const TSqlRelation &relation)
  {
  if (isValidRelation(relation) && m_relations.contains(relation)==false)
    m_relations.append(relation);
  }

void TSqlTableModel::removeRelation(const TSqlRelation &relation)
  {
  if (m_relations.contains(relation))
    m_relations.removeAll(relation);
  }

QList<TSqlRelation> TSqlTableModel::relations() const
  {
  return m_relations;
  }

bool TSqlTableModel::isValidRelation(const TSqlRelation &relation) const
  {
  QString relatedTable=relation.relatedTable();
  if (record().contains(relation.field())==false || database().tables().contains(relatedTable)==false)
    return false;

  QSqlRecord record=database().record(relatedTable);
  return record.contains(relation.indexField()) && record.contains(relation.relatedField());
  }
