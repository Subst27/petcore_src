#pragma once

#include <QStyledItemDelegate>

class TDataDelegate : public QStyledItemDelegate
  {
    Q_OBJECT
  public:
    explicit TDataDelegate(QObject *parent = nullptr);

  protected:
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
    bool editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index);
    //QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const;
  };
