#pragma once

#include "TSqlTableModel.h"

class TDoctorsModel : public TSqlTableModel
  {
    Q_OBJECT
  public:
    explicit TDoctorsModel(QObject *parent=nullptr, const QSqlDatabase &dataBase=QSqlDatabase());
    void setHeaderNames() override;

  protected:
    QString selectStatement() const override;

  };
