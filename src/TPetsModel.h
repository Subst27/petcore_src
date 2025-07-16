#pragma once

#include "TSqlTableModel.h"

class TPetsModel : public TSqlTableModel
  {
    Q_OBJECT
  public:
    explicit TPetsModel(QObject *parent=nullptr, const QSqlDatabase &dataBase=QSqlDatabase());
    void setHeaderNames() override;

  protected:
    QString selectStatement() const;

  };
