#pragma once

#include "TSqlTableModel.h"

class TProfilesModel : public TSqlTableModel
  {
    Q_OBJECT
  public:
    explicit TProfilesModel(QObject *parent=nullptr, const QSqlDatabase &dataBase=QSqlDatabase());
    void setHeaderNames() override;

  protected:
    QString selectStatement() const override;

  };
