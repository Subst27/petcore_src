#include "TComboBox.h"

#include "TSqlTableModel.h"

#include <QAbstractItemView>
#include <QFontMetrics>
#include <QDebug>
#include <QScrollBar>
#include <QLineEdit>
#include <QKeyEvent>

//#include <QSqlTableModel>
#include <QSqlRecord>

TComboBox::TComboBox(QWidget *parent) : QComboBox(parent), m_readOnly(false)
  {

  }

void TComboBox::showPopup()
  {
  if (m_readOnly==true)
    return;

  QComboBox::showPopup();
  quint16 maxWidth=0;
  quint16 width;

  QFontMetrics metrics(font());

  for (quint16 i=0;i<count();i++)
    {
    width=metrics.horizontalAdvance(itemText(i));
    if (width>maxWidth)
      maxWidth=width;
    }

  QFrame *popupFrame=qobject_cast<QFrame*>(view()->parent());
  QRect frameRect=popupFrame->geometry();
  if (frameRect.width()<maxWidth+view()->horizontalScrollBar()->height())
    frameRect.setWidth(maxWidth+view()->horizontalScrollBar()->height());

  popupFrame->setGeometry(frameRect);
  emit popupShown();
  }

void TComboBox::hidePopup()
  {
  QComboBox::hidePopup();
  emit popupHidden();
  }

void TComboBox::keyReleaseEvent(QKeyEvent *event)
  {
  if (event->key()==Qt::Key_Escape)
    {
    setCurrentIndex(-1);
    emit activated(-1);
    }
  else
    QComboBox::keyReleaseEvent(event);

  hidePopup();
  emit keyPressed();
  }

void TComboBox::setModel(TSqlTableModel *model, const QStringList &valueFields, const QString &indexField, const QString &filter)
  {
  clear();
  QString currentFilter=model->filter();
  if (filter.isEmpty()==false)
    model->setFilter(filter); // NOTE: не заменить фильтр, а дополнить?

  m_data.clear();
  m_data.insert(-1,QMap<QString,QVariant>());

  for (quint32 i=0;i<model->rowCount();i++)
    {
    QSqlRecord record=model->record(i);

    QMap<QString,QVariant> map;
    QStringList values;
    for (quint16 i=0;i<valueFields.size();i++)
      {
      QString string=record.value(valueFields.at(i)).toString();
      values << string;
      map.insert(valueFields.at(i),string);
      }

    /*addItem(value.join("; "),record.value(indexField));*/
    /* первое из списка поле считаем титульным */
    addItem(values.first(),record.value(indexField));
    m_data.insert(i, map);
    }

  model->setFilter(currentFilter);
  setCurrentIndex(-1);
  }

QMap<QString, QVariant> TComboBox::data(qint32 index)
  {
  return m_data.value(index);
  }

void TComboBox::setReadOnly(bool readOnly)
  {
  m_readOnly=readOnly;
  }
