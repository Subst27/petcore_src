#pragma once

#include "TSqlTableModel.h"

class TClientsModel : public TSqlTableModel
  {
    Q_OBJECT
  public:
    explicit TClientsModel(QObject *parent=nullptr, const QSqlDatabase &dataBase=QSqlDatabase());
    void setHeaderNames() override;

  protected:
    QString selectStatement() const;

  };
