#pragma once

#include "TSqlTableModel.h"

class TStatusesModel : public TSqlTableModel
  {
    Q_OBJECT
  public:
    explicit TStatusesModel(QObject *parent=nullptr, const QSqlDatabase &dataBase=QSqlDatabase());
    void setHeaderNames() override;

  protected:
    QString selectStatement() const;

  };
