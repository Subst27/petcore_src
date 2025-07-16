#include "TRoutinesDelegate.h"
#include "TRoutinesModel.h"
#include "TRoutinesFilterModel.h"

#include <QApplication>
#include <QTimeEdit>

TRoutinesDelegate::TRoutinesDelegate(QObject *parent) : QStyledItemDelegate{parent}
  {

  }

void TRoutinesDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
  {
  const TRoutinesFilterModel *filterModel=qobject_cast<const TRoutinesFilterModel*>(index.model());
  if (filterModel==nullptr)
    return;

  TRoutinesModel *routinesModel=qobject_cast<TRoutinesModel*>(filterModel->sourceModel());
  if (routinesModel==nullptr)
    return;

  QStyleOptionViewItem itemOption(option);

  QTime fromAm=routinesModel->index(index.row(),routinesModel->fieldIndex("from_am")).data().toTime();
  QTime toAm=routinesModel->index(index.row(),routinesModel->fieldIndex("to_am")).data().toTime();
  QTime fromPm=routinesModel->index(index.row(),routinesModel->fieldIndex("from_pm")).data().toTime();
  QTime toPm=routinesModel->index(index.row(),routinesModel->fieldIndex("to_pm")).data().toTime();
  if (fromAm==toAm && fromPm==toPm)
    itemOption.backgroundBrush=QColor("#D9E5EC");

  quint8 weekDay=routinesModel->index(index.row(),routinesModel->fieldIndex("day")).data().toUInt() % 7;
  if (weekDay == 5) // суббота
    itemOption.backgroundBrush=QColor("#E0DBFF");

  if (weekDay == 6) // воскресенье
    itemOption.backgroundBrush=QColor("#ABBDFF");

  if (index.column()==routinesModel->fieldIndex("day"))
    {
    QStringList weekDays={tr("Monday"),tr("Tuesday"),tr("Wednesday"),tr("Thursday"),tr("Friday"),tr("Saturday"),tr("Sunday")};
    itemOption.text=weekDays.at(weekDay);
    itemOption.displayAlignment=Qt::AlignVCenter | Qt::AlignLeft;
    }
  else
    {
    itemOption.text=index.data().toTime().toString("HH:mm");
    itemOption.displayAlignment=Qt::AlignVCenter | Qt::AlignRight;
    }

  qApp->style()->drawControl(QStyle::CE_ItemViewItem,&itemOption,painter);
  }

QWidget *TRoutinesDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const
  {
  const TRoutinesFilterModel *filterModel=qobject_cast<const TRoutinesFilterModel*>(index.model());
  if (filterModel==nullptr)
    return nullptr;

  TRoutinesModel *routineModel=qobject_cast<TRoutinesModel*>(filterModel->sourceModel());
  if (routineModel==nullptr)
    return nullptr;

  // day нельзя редактировать ручками
  if (index.column()==routineModel->fieldIndex("day"))
    return nullptr;

  QTimeEdit *timeEdit=new QTimeEdit(parent);
  timeEdit->setDisplayFormat("HH:mm");
  timeEdit->setAlignment(Qt::AlignVCenter | Qt::AlignRight);

  return timeEdit;
  }

void TRoutinesDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
  {
  QTimeEdit *timeEdit=qobject_cast<QTimeEdit*> (editor);
  if (timeEdit==nullptr)
    return;

  timeEdit->setTime(index.data().toTime());
  }

void TRoutinesDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
  {
  QTimeEdit *timeEdit=qobject_cast<QTimeEdit*> (editor);
  if (timeEdit==nullptr)
    return;

  model->setData(index,timeEdit->time().toString("HH:mm:ss"));
  }
