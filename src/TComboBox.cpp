#include "TComboBox.h"

#include "TSqlTableModel.h"
#include "TComboListView.h"

#include <QDebug>

#include <QAbstractItemView>
#include <QFontMetrics>
#include <QScrollBar>
#include <QLineEdit>
#include <QKeyEvent>
#include <QApplication>

#include <QSqlRecord>

TComboBox::TComboBox(QWidget *parent) : QComboBox(parent), m_readOnly(false), m_popupShown(false)
  {
  TComboListView *listView=new TComboListView(this);
  setView(listView);
  }

void TComboBox::showPopup()
  {
  if (m_readOnly==true || m_popupShown==true)
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
  m_popupShown=true;

  emit popupShown();
  }

void TComboBox::hidePopup()
  {
  if (m_popupShown==false)
    return;

  QComboBox::hidePopup();
  m_popupShown=false;

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

void TComboBox::listViewKeyPressed(QKeyEvent *event)
  {
  QString text=lineEdit()->text();
  clear();
  lineEdit()->setText(text);

  QKeyEvent pressEvent(QKeyEvent::KeyPress,event->key(),event->modifiers(),event->text());
  keyPressEvent(&pressEvent);

  QKeyEvent releaseEvent(QKeyEvent::KeyRelease,event->key(),event->modifiers(),event->text());
  keyReleaseEvent(&releaseEvent);
  }

void TComboBox::setModel(TSqlTableModel *model, const QStringList &valueFields, const QString &indexField, const QString &filter)
  {
  clear();
  QString currentFilter=model->filter();
  if (filter.isEmpty()==false)
    model->setFilter(filter);

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

void TComboBox::setReplaceKeyEvent(bool replace)
  {
  TComboListView *listView=qobject_cast<TComboListView*>(view());
  if (listView==nullptr)
    return;

  if (replace==true)
    connect(listView,&TComboListView::keyPressed,this,&TComboBox::listViewKeyPressed,Qt::UniqueConnection);
  else
    disconnect(listView,&TComboListView::keyPressed,this,&TComboBox::listViewKeyPressed);
  }
