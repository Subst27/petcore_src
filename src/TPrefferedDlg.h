#pragma once

#include "ui_TPrefferedDlg.h"

class TPrefferedDlg : public QDialog, private Ui::TPrefferedDlg
  {
    Q_OBJECT

  public:
    explicit TPrefferedDlg(const QString &preffered, QWidget *parent = nullptr);
    QString preffered() const;

  protected:
    void writeSettings();
    void readSettings();

  protected slots:
    void accept();
    void reject();
  };

