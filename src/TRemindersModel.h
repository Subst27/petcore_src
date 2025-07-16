#pragma once

#include "TSqlTableModel.h"

class TRemindersModel : public TSqlTableModel
  {
    Q_OBJECT
  public:
    explicit TRemindersModel(QObject *parent=nullptr, const QSqlDatabase &dataBase=QSqlDatabase());
    void setHeaderNames() override;

  protected:
    QString selectStatement() const;

  };
