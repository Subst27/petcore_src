#include "TApptsFilterModel.h"

TApptsFilterModel::TApptsFilterModel(QObject *parent) : QSortFilterProxyModel{parent},
  m_fromDateTime(QDateTime()), m_toDateTime(QDateTime()), m_states({})
  {
  setDynamicSortFilter(false);
  }

void TApptsFilterModel::setFilterData(const QDateTime &fromDateTime, const QDateTime &toDateTime, QList <TApptsModel::States> states)
  {
  m_fromDateTime=fromDateTime;
  m_toDateTime=toDateTime;

  m_states=states;

  invalidateFilter();
  }

bool TApptsFilterModel::filterAcceptsRow(int row, const QModelIndex &parent) const
  {
  TApptsModel *apptsModel=qobject_cast<TApptsModel*>(sourceModel());
  if (apptsModel==nullptr)
    return false;

  QDate date=apptsModel->index(row,apptsModel->fieldIndex("appt_date"),parent).data().toDate();
  QTime time=apptsModel->index(row,apptsModel->fieldIndex("appt_time"),parent).data().toTime();

  TApptsModel::States state=(TApptsModel::States)apptsModel->index(row,apptsModel->fieldIndex("state"),parent).data().toUInt();

  bool accept=true;
  if (m_fromDateTime.isValid()==true)
    accept &= (QDateTime(date,time)>=m_fromDateTime);

  if (m_toDateTime.isValid()==true)
    accept &= (QDateTime(date,time)<m_toDateTime);

  if (m_states.isEmpty()==false)
    accept &= m_states.contains(state);

  return accept;
  }
