#include "TComboListView.h"

#include <QDebug>

TComboListView::TComboListView(QWidget *parent) : QListView(parent)
  {

  }

void TComboListView::keyReleaseEvent(QKeyEvent *event)
  {
  emit keyPressed(event);
  }
