#pragma once

#include "TSqlTableModel.h"

class TRanksModel : public TSqlTableModel
  {
    Q_OBJECT
  public:
    explicit TRanksModel(QObject *parent=nullptr, const QSqlDatabase &dataBase=QSqlDatabase());
    void setHeaderNames() override;

  protected:
    QString selectStatement() const override;

  };
