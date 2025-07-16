#pragma once

#include "TSqlTableModel.h"

class TBreedsModel : public TSqlTableModel
  {
    Q_OBJECT
  public:
    explicit TBreedsModel(QObject *parent=nullptr, const QSqlDatabase &dataBase=QSqlDatabase());
    void setHeaderNames() override;

  protected:
    QString selectStatement() const;

  private:

  };
