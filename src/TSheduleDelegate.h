#pragma once

#include <QStyledItemDelegate>

class TSheduleDelegate : public QStyledItemDelegate
  {
    Q_OBJECT
  public:
    explicit TSheduleDelegate(QObject *parent = nullptr);

  protected:
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override;

  private:
    /* TSheduleModel *m_sheduleModel;
     * TRoutinesModel *m_routinesModel;
     * TApptsModel *m_apptsModel; */
  };
