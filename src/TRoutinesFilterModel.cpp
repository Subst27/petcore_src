#include "TRoutinesFilterModel.h"
#include "TRoutinesModel.h"

#include <QTime>
#include <QDebug>

TRoutinesFilterModel::TRoutinesFilterModel(TRoutinesFilterModel::Types type, QObject *parent) : QSortFilterProxyModel{parent},
  m_type(type), m_day(-1)
  {
  setDynamicSortFilter(false);
  }

void TRoutinesFilterModel::setFilterData(qint8 day)
  {
  m_day=day;
  invalidate();
  }

bool TRoutinesFilterModel::filterAcceptsRow(int row, const QModelIndex &parent) const
  {
  TRoutinesModel *routineModel=qobject_cast<TRoutinesModel*>(sourceModel());
  if (routineModel==nullptr)
    return false;

  quint8 day=routineModel->index(row,routineModel->fieldIndex("day"),parent).data().toUInt();

  bool accept=true;
  if (m_day>-1)
    accept &= (day==m_day);

  quint8 remainder=(day / 7) % 2;
  // если фильтр безотносительно четной/нечетной недели, а для получения расписания доктора за определенный день, если принимает
  if (m_type==TRoutinesFilterModel::Available)
    {
    QTime fromAm=routineModel->index(row,routineModel->fieldIndex("from_am"),parent).data().toTime();
    QTime toAm=routineModel->index(row,routineModel->fieldIndex("to_am"),parent).data().toTime();
    QTime fromPm=routineModel->index(row,routineModel->fieldIndex("from_pm"),parent).data().toTime();
    QTime toPm=routineModel->index(row,routineModel->fieldIndex("to_pm"),parent).data().toTime();

    return (accept && (fromAm<toAm || fromPm<toPm));
    }

  // если нечетная (первая) неделя
  if (m_type==TRoutinesFilterModel::Odd)
    return (accept && remainder==0);

  // если четная (вторая) неделя
  return (accept && remainder==1);
  }
