#pragma once

#include "TSqlTableModel.h"

class TApptsModel : public TSqlTableModel
  {
    Q_OBJECT
  public:
    enum States : quint8
      {
      Created,
      Started,
      Finished
      };
    Q_ENUM(States)

    explicit TApptsModel(QObject *parent=nullptr, const QSqlDatabase &dataBase=QSqlDatabase());
    void setHeaderNames() override;

    static QString textByState(TApptsModel::States state);

  protected:
    QString selectStatement() const override;

  };
