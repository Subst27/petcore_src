#pragma once

#include <QStyledItemDelegate>

class TRemindersDelegate : public QStyledItemDelegate
  {
  public:
    explicit TRemindersDelegate(QObject *parent = nullptr);
    void setReminderDeadline(quint16 deadline);
    quint16 deadline();

  protected:
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
    bool editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index);

  private:
    quint16 m_deadline;
  };
