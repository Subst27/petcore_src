#pragma once

#include "TSqlTableModel.h"

class TDegreesModel : public TSqlTableModel
  {
    Q_OBJECT
  public:
    explicit TDegreesModel(QObject *parent=nullptr, const QSqlDatabase &dataBase=QSqlDatabase());
    void setHeaderNames() override;

  protected:
    QString selectStatement() const;

  };
