#pragma once

#include "TSqlTableModel.h"

class TSpeciesModel : public TSqlTableModel
  {
    Q_OBJECT
  public:
    explicit TSpeciesModel(QObject *parent=nullptr, const QSqlDatabase &dataBase=QSqlDatabase());
    void setHeaderNames() override;

  protected:
    QString selectStatement() const override;

  };
