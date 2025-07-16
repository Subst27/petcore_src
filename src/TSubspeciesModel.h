#pragma once

#include "TSqlTableModel.h"

class TSubspeciesModel : public TSqlTableModel
  {
    Q_OBJECT
  public:
    explicit TSubspeciesModel(QObject *parent=nullptr, const QSqlDatabase &dataBase=QSqlDatabase());
    void setHeaderNames() override;

  protected:
    QString selectStatement() const override;

  };
