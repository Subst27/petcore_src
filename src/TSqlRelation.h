#pragma once

#include <QDebug>

class TSqlRelation
  {
    // смысл такой (field - ключевое поле, visibleField - отображаемое поле,
    // relatedTable - связанная таблица, indexField - индексное поле в связанной таблице, relatedField - возвращаемое поле в связанной таблице
    // т.е. select table.field, relatedTable.relatedField as table.visibleField from table left join realtedTable on table.field=relatedTable.indexField
  public:
    explicit TSqlRelation();
    explicit TSqlRelation(const QString &field, const QString &visibleField, const QString &relatedTable, const QString &indexField, const QString &relatedField);

#ifndef QT_NO_DEBUG_STREAM
    friend QDebug operator << (QDebug debug,const TSqlRelation &relation)
      {
      debug.nospace()<<"TSqlRelation("<<relation.field()<<","<<relation.visibleField()<<","<<relation.relatedTable()<<","<<relation.indexField()<<","<<relation.relatedField()<<")";
      return debug.space();
      }
#endif

    void setField(const QString &filed);
    QString field() const;

    void setVisibleField(const QString &visibleField);
    QString visibleField() const;

    void setRelatedTable(const QString &relatedTable);
    QString relatedTable() const;

    void setIndexField(const QString &indexField);
    QString indexField() const;

    void setRelatedField(const QString &relatedField);
    QString relatedField() const;

    bool operator ==(const TSqlRelation &other) const;
    bool operator !=(const TSqlRelation &other) const;

  private:
    QString m_field;
    QString m_visibleField;
    QString m_relatedTable;
    QString m_indexField;
    QString m_relatedField;
  };
