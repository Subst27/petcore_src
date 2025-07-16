#pragma once

#include "TSqlTableModel.h"

class TTasksModel : public TSqlTableModel
  {
    Q_OBJECT
  public:
    explicit TTasksModel(QObject *parent=nullptr, const QSqlDatabase &dataBase=QSqlDatabase());
    void setHeaderNames() override;

  protected:
    QString selectStatement() const override;

  };
