#pragma once

#include "ui_TLoginDlg.h"

class QRegularExpressionValidator;

class TLoginDlg : public QDialog, private Ui::TLoginDlg
  {
    Q_OBJECT

  public:
    explicit TLoginDlg(QWidget *parent = nullptr);
    QPair<QString,QString> credentials() const;

  protected:
    void showEvent(QShowEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;
    void writeSettings();
    void readSettings();

  protected slots:
    void changeEchoMode(bool checked);
    void loginPositionChanged(int oldPos, int newPos);

    void accept();
    void reject();
  };

