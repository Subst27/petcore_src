#pragma once

#include "TSqlTableModel.h"

class TActionsModel : public TSqlTableModel
  {
    Q_OBJECT
  public:
    explicit TActionsModel(QObject *parent=nullptr, const QSqlDatabase &dataBase=QSqlDatabase());
    void setHeaderNames() override;

  protected:
    QString selectStatement() const override;

  };
