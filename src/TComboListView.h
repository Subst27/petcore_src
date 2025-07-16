#pragma once

#include <QListView>

class TComboListView : public QListView
  {
    Q_OBJECT
  public:
    explicit TComboListView(QWidget *parent=nullptr);

  protected:
    void keyReleaseEvent(QKeyEvent *event) override;

  signals:
    void keyPressed(QKeyEvent *event);
  };
