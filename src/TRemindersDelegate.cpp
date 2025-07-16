#include "TRemindersDelegate.h"
#include "TSqlTableModel.h"
#include "TSettings.h"

#include <QToolTip>
#include <QDate>
#include <QMouseEvent>

#include <QSqlRecord>
#include <QSqlField>

TRemindersDelegate::TRemindersDelegate(QObject *parent) : QStyledItemDelegate{parent}, m_deadline(3)
  {

  }

void TRemindersDelegate::setReminderDeadline(quint16 deadline)
  {
  m_deadline=deadline;
  }

quint16 TRemindersDelegate::deadline()
  {
  return m_deadline;
  }

void TRemindersDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
  {
  const TSqlTableModel *tableModel=qobject_cast<const TSqlTableModel*>(index.model());
  QString fieldName=tableModel->record().fieldName(index.column());

  QStyleOptionViewItem itemOption(option);

  if (fieldName=="reminder_date")
    {
    itemOption.text=index.data().toDate().toString("dd.MM.yyyy");
    itemOption.displayAlignment=Qt::AlignVCenter | Qt::AlignRight;
    }
  else
    {
    itemOption.text=index.data().toString();
    itemOption.displayAlignment=Qt::AlignVCenter | Qt::AlignLeft;
    }


  QDate today=QDate::currentDate();
  QDate reminderDate=tableModel->index(index.row(),tableModel->fieldIndex("reminder_date")).data().toDate();

  // если настало время напомнить
  if (today.daysTo(reminderDate)<m_deadline+1)
    itemOption.backgroundBrush=QColor("#FF8080");

  // если напоминание уже "протухла"
  if (today.daysTo(reminderDate)<0)
    itemOption.backgroundBrush=QColor("#F1A3FF");

  qApp->style()->drawControl(QStyle::CE_ItemViewItem,&itemOption,painter);
  }

bool TRemindersDelegate::editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index)
  {
  if (event->type()!=QEvent::MouseButtonPress)
    return false;

  QMouseEvent *mouseEvent=static_cast<QMouseEvent*>(event);
  QToolTip::showText(mouseEvent->globalPos(), index.data().toString());

  return false;
  }
