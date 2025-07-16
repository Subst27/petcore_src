#pragma once

#include "ui_TPeriodDlg.h"

class TPeriodDlg : public QDialog, private Ui::TPeriodDlg
  {
    Q_OBJECT

  public:
    explicit TPeriodDlg(QWidget *parent = nullptr);
    QPair<QDate,QDate> period() const;

  protected:
    virtual void writeSettings();
    virtual void readSettings();

  protected slots:
    void accept();
    void reject();
  };

