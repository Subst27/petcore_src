#include "TTasksDelegate.h"

#include "TSqlTableModel.h"
#include "TSettings.h"

#include "TPalette.h"

#include <QToolTip>
#include <QDate>
#include <QMouseEvent>

#include <QSqlRecord>
#include <QSqlField>

TTasksDelegate::TTasksDelegate(QObject *parent) : QStyledItemDelegate{parent}, m_deadline(3)
  {
  /* задача завершена       - TPalette::Unavailable
   * пора напоминать        - TPalette::Warning
   * напоминание протухло   - TPalette::Missed */
  }

void TTasksDelegate::setReminderDeadline(quint16 deadline)
  {
  m_deadline=deadline;
  }

quint16 TTasksDelegate::deadline()
  {
  return m_deadline;
  }

void TTasksDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
  {
  const TSqlTableModel *tableModel=qobject_cast<const TSqlTableModel*>(index.model());
  QString fieldName=tableModel->record().fieldName(index.column());
  QStringList dateFields={"task_date","reminder_date"};

  QStyleOptionViewItem itemOption(option);

  if (dateFields.contains(fieldName)==true)
    {
    itemOption.text=index.data().toDate().toString("dd.MM.yyyy");
    itemOption.displayAlignment=Qt::AlignVCenter | Qt::AlignRight;
    }
  else
    {
    itemOption.text=index.data().toString();
    itemOption.displayAlignment=Qt::AlignVCenter | Qt::AlignLeft;
    }

  /* это для задач */
  if (tableModel->fieldIndex("completed")>-1 && tableModel->index(index.row(),tableModel->fieldIndex("completed")).data().toBool()==true)
    {
    itemOption.backgroundBrush=TPalette::color(TPalette::Unavailable,TPalette::Background);
    itemOption.palette.setColor(QPalette::Text,TPalette::color(TPalette::Unavailable,TPalette::Foregroud));
    }

  /* это для напоминалок */
  if (tableModel->fieldIndex("reminder_date")>-1)
    {
    QDate today=QDate::currentDate();
    QDate reminderDate=tableModel->index(index.row(),tableModel->fieldIndex("reminder_date")).data().toDate();
    // если настало время напомнить
    if (reminderDate.isValid()==true && today.daysTo(reminderDate)<m_deadline+1)
      {
      itemOption.backgroundBrush=TPalette::color(TPalette::Warning,TPalette::Background);
      itemOption.palette.setColor(QPalette::Text,TPalette::color(TPalette::Warning,TPalette::Foregroud));
      }

    // если напоминание уже "протухла"
    if (reminderDate.isValid()==true && today.daysTo(reminderDate)<0)
      {
      itemOption.backgroundBrush=TPalette::color(TPalette::Missed,TPalette::Background);
      itemOption.palette.setColor(QPalette::Text,TPalette::color(TPalette::Missed,TPalette::Foregroud));
      }
    }

  qApp->style()->drawControl(QStyle::CE_ItemViewItem,&itemOption,painter);
  }

bool TTasksDelegate::editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index)
  {
  if (event->type()!=QEvent::MouseButtonPress)
    return false;

  QMouseEvent *mouseEvent=static_cast<QMouseEvent*>(event);
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
  QToolTip::showText(mouseEvent->globalPosition().toPoint(), index.data().toString());
#else
  QToolTip::showText(mouseEvent->globalPos(), index.data().toString());
#endif

  return false;
  }

/*QSize TTasksDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
  {
  Q_UNUSED(index)
  return QSize(option.rect.width(),qApp->font().pointSize()*2.5);
  }*/
