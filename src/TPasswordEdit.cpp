#include "TPasswordEdit.h"

#include <QPushButton>
#include <QResizeEvent>

TPasswordEdit::TPasswordEdit(QWidget *parent) : QLineEdit(parent)
  {
  m_button=new QPushButton(this);
  m_button->setCursor(Qt::ArrowCursor);
  m_button->setCheckable(true);
  m_button->setChecked(false);
  // NOTE: достать префик темы из qApp->property("appt_theme").toString()
  m_button->setIcon(QPixmap(":/images/eye_on"));
  m_button->setStyleSheet("padding: 0px; margin: 0px; border: none;");

  connect(m_button,&QPushButton::clicked,this,&TPasswordEdit::changeDisplayMode);
  }

void TPasswordEdit::resizeEvent(QResizeEvent *event)
  {
  m_button->resize(height(),height());
  m_button->move(width()-height(),0);
  event->accept();
  }

void TPasswordEdit::changeDisplayMode(bool checked)
  {
  if (checked==true)
    {
    // NOTE: достать префик темы из qApp->property("appt_theme").toString()
    m_button->setIcon(QPixmap(":/images/eye_off"));
    setEchoMode(QLineEdit::Normal);
    }
  else
    {
    // NOTE: достать префик темы из qApp->property("appt_theme").toString()
    m_button->setIcon(QPixmap(":/images/eye_on"));
    setEchoMode(QLineEdit::Password);
    }
  }
