#include "TDataDelegate.h"

#include "TClientsModel.h"
#include "TPetsModel.h"

#include <QApplication>
#include <QSqlRecord>
#include <QSqlField>

#include <QDate>
#include <QPainter>
#include <QDebug>

#include <QToolTip>
#include <QMouseEvent>

TDataDelegate::TDataDelegate(QObject *parent) : QStyledItemDelegate{parent}
  {

  }

void TDataDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
  {
  const TSqlTableModel *tableModel=qobject_cast<const TSqlTableModel*>(index.model());
  if (tableModel==nullptr)
    {
    QStyledItemDelegate::paint(painter,option,index);
    return;
    }

  QString fieldName=tableModel->record().fieldName(index.column());

  QStyleOptionViewItem itemOption(option);
  // какие поля (во всех таблицах) выравнивать по правому краю
  QStringList rightAllignFields={"passport", "phone_number", "birth_date", "marking_date", "vet_passport", "uicmm", "certificate_date", "experience", "interval"};
  if (rightAllignFields.contains(fieldName)==true)
    itemOption.displayAlignment=Qt::AlignVCenter | Qt::AlignRight;
  else
    itemOption.displayAlignment=Qt::AlignVCenter | Qt::AlignLeft;

  // поля с датами
  QStringList dateFields={"birth_date", "marking_date", "certificate_date"};
  if (dateFields.contains(fieldName))
    itemOption.text=index.data().toDate().toString("dd.MM.yyyy");
  else
    itemOption.text=index.data().toString();

  if (fieldName=="gender")
    itemOption.text=index.data().toBool()==false ? tr("Female") : tr("Male");

  if (tableModel->fieldIndex("forbidden")>-1 && tableModel->index(index.row(),tableModel->fieldIndex("forbidden")).data().toBool()==true)
    itemOption.backgroundBrush=QColor("#D9E5EC");

  if (tableModel->fieldIndex("available")>-1)
    {
    bool available=tableModel->index(index.row(),tableModel->fieldIndex("available")).data().toBool();
    if (fieldName=="available")
      itemOption.text=index.data().toBool()==false ? tr("No") : tr("Yes");

    if (available==false)
      itemOption.backgroundBrush=QColor("#D9E5EC");
    }

  qApp->style()->drawControl(QStyle::CE_ItemViewItem,&itemOption,painter);
  }

bool TDataDelegate::editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index)
  {
  if (event->type()!=QEvent::MouseButtonPress)
    return false;

  QMouseEvent *mouseEvent=static_cast<QMouseEvent*>(event);
  QToolTip::showText(mouseEvent->globalPos(), index.data().toString());

  return false;
  }
