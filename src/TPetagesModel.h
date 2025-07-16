#pragma once

#include "TSqlTableModel.h"

class TPetagesModel : public TSqlTableModel
  {
    Q_OBJECT
  public:
    explicit TPetagesModel(QObject *parent=nullptr, const QSqlDatabase &dataBase=QSqlDatabase());
    void setHeaderNames() override;

  protected:
    QString selectStatement() const;

  };
