#include "TSqlRelation.h"

TSqlRelation::TSqlRelation() : m_field(QString()), m_visibleField(QString()), m_relatedTable(QString()), m_indexField(QString()), m_relatedField(QString())
  {

  }

TSqlRelation::TSqlRelation(const QString &field, const QString &visibleField, const QString &relatedTable, const QString &indexField, const QString &relatedField) :
  m_field(field), m_visibleField(visibleField), m_relatedTable(relatedTable), m_indexField(indexField), m_relatedField(relatedField)
  {

  }

void TSqlRelation::setField(const QString &filed)
  {
  if (m_field!=filed)
    m_field=filed;
  }

QString TSqlRelation::field() const
  {
  return m_field;
  }

void TSqlRelation::setVisibleField(const QString &visibleField)
  {
  if (m_visibleField!=visibleField)
    m_visibleField=visibleField;
  }

QString TSqlRelation::visibleField() const
  {
  return m_visibleField;
  }

void TSqlRelation::setRelatedTable(const QString &relatedTable)
  {
  if (m_relatedTable!=relatedTable)
    m_relatedTable=relatedTable;
  }

QString TSqlRelation::relatedTable() const
  {
  return m_relatedTable;
  }

void TSqlRelation::setIndexField(const QString &indexField)
  {
  if (m_indexField!=indexField)
    m_indexField=indexField;
  }

QString TSqlRelation::indexField() const
  {
  return m_indexField;
  }

void TSqlRelation::setRelatedField(const QString &relatedField)
  {
  if (m_relatedField!=relatedField)
    m_relatedField=relatedField;
  }

QString TSqlRelation::relatedField() const
  {
  return m_relatedField;
  }

bool TSqlRelation::operator ==(const TSqlRelation &other) const
  {
  return (m_field==other.field() && m_visibleField==other.visibleField() &&
          m_relatedTable==other.relatedTable() && m_indexField==other.indexField() && m_relatedField==other.relatedField());
  }

bool TSqlRelation::operator !=(const TSqlRelation &other) const
  {
  return (m_field!=other.field() || m_visibleField!=other.visibleField() ||
          m_relatedTable!=other.relatedTable() || m_indexField!=other.indexField() || m_relatedField!=other.relatedField());
  }
