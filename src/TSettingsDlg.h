#pragma once

#include "ui_TSettingsDlg.h"

class TSettingsDlg : public QDialog, private Ui::TSettingsDlg
  {
    Q_OBJECT

  public:
    explicit TSettingsDlg(QWidget *parent = nullptr);

  protected:
    void writeSettings();
    void readSettings();

  protected slots:
    void accept();
    void reject();

    void selectFont();
  };
