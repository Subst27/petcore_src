#pragma once

#include "ui_TAboutDlg.h"

class TAboutDlg : public QDialog, private Ui::TAboutDlg
  {
    Q_OBJECT

  public:
    explicit TAboutDlg(QWidget *parent = nullptr);
  };
