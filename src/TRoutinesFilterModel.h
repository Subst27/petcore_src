#pragma once

#include <QSortFilterProxyModel>

class TRoutinesFilterModel : public QSortFilterProxyModel
  {
    Q_OBJECT
  public:
    enum Types
      {
      Available=0, // есть прием в этот день
      Odd,         // нечетная неделя
      Even         // четная нееделя
      };
    Q_ENUM(Types)

    explicit TRoutinesFilterModel(TRoutinesFilterModel::Types type, QObject *parent = nullptr);
    void setFilterData(qint8 day);

  protected:
    bool filterAcceptsRow(int row, const QModelIndex &parent) const override;

  private:
    TRoutinesFilterModel::Types m_type;
    qint8 m_day;
  };

