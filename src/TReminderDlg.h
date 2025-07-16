#pragma once

#include "ui_TReminderDlg.h"
#include "TModelDlg.h"

class TReminderDlg : public TModelDlg, private Ui::TReminderDlg
  {
    Q_OBJECT

  public:
    explicit TReminderDlg(const QString &clientName, QWidget *parent = nullptr);
    QSqlRecord record() const;

  protected slots:
    void accept();

  };
