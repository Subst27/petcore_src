#pragma once

#include "ui_TAboutDlg.h"

class TAboutDlg : public QDialog, private Ui::TAboutDlg
  {
    Q_OBJECT

  public:
    explicit TAboutDlg(QWidget *parent = nullptr);

  protected:
    void showEvent(QShowEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;
    void writeSettings();
    void readSettings();

  protected slots:
    void reject() override;
  };
