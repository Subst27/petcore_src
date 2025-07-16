#pragma once

#include "TSqlTableModel.h"

class TMarkingModel : public TSqlTableModel
  {
    Q_OBJECT
  public:
    explicit TMarkingModel(QObject *parent=nullptr, const QSqlDatabase &dataBase=QSqlDatabase());
    void setHeaderNames() override;

  protected:
    QString selectStatement() const;

  };
