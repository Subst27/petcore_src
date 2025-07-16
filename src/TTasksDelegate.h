#pragma once

#include <QStyledItemDelegate>

class TTasksDelegate : public QStyledItemDelegate
  {
  public:
    explicit TTasksDelegate(QObject *parent = nullptr);
    void setReminderDeadline(quint16 deadline);
    quint16 deadline();

  protected:
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
    bool editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index);
    //QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const;

  private:
    quint16 m_deadline;
  };
