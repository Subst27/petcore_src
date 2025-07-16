#pragma once

#include "TSqlTableModel.h"

class TRoutinesModel : public TSqlTableModel
  {
    Q_OBJECT
  public:
    explicit TRoutinesModel(QObject *parent=nullptr, const QSqlDatabase &dataBase=QSqlDatabase());
    void setHeaderNames() override;

  protected:
    QString selectStatement() const override;

  };
