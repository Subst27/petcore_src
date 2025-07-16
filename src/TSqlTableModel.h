#pragma once

#include <QSqlTableModel>

#include "TSqlRelation.h"

class TSqlTableModel : public QSqlTableModel
  {
    Q_OBJECT
  public:
    explicit TSqlTableModel(QObject *parent=nullptr, const QSqlDatabase &dataBase=QSqlDatabase());

    void setTitle(const QString &title);
    QString title() const;

    void setHeaderField(const QString &field);
    QString headerField() const;

    void setRelations(const QList <TSqlRelation> &relations);
    void clearRelations();
    void appendRelation(const TSqlRelation &relation);
    void removeRelation(const TSqlRelation &relation);
    QList<TSqlRelation> relations() const;

    bool isValidRelation(const TSqlRelation &relation) const;
    virtual void setHeaderNames()=0;

  public slots:
    bool select() override;
    bool clearModel();
    /*bool removeRows(int row, int count, const QModelIndex &parent) override;
    bool removeRow(int row, const QModelIndex &parent=QModelIndex());*/

  private:
    QString m_title;
    QString m_headerField;
    QList <TSqlRelation> m_relations;

  signals:
    void titleChanged(const QString &title);
    void headerFieldChanged(const QString &headerField);
  };
