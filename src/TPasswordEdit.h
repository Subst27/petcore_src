#pragma once

#include <QLineEdit>

class QPushButton;

class TPasswordEdit : public QLineEdit
  {
    Q_OBJECT
  public:
    explicit TPasswordEdit(QWidget *parent);

  protected:
    void resizeEvent(QResizeEvent *);

  private:
    QPushButton *m_button;

  protected slots:
    void changeDisplayMode(bool checked);
  };
