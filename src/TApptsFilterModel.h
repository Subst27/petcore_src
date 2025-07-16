#pragma once

#include <QSortFilterProxyModel>
#include <QDate>
#include <QTime>

#include "TApptsModel.h"

class TApptsFilterModel : public QSortFilterProxyModel
  {
    Q_OBJECT
  public:
    explicit TApptsFilterModel(QObject *parent = nullptr);
    void setFilterData(const QDateTime &fromDateTime, const QDateTime &toDateTime, QList <TApptsModel::States> states);

  protected:
    bool filterAcceptsRow(int row, const QModelIndex &parent) const;

  private:
    QDateTime m_fromDateTime;
    QDateTime m_toDateTime;

    QList <TApptsModel::States> m_states;
  };

