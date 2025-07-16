#pragma once

#include "ui_TTaskDlg.h"
#include "TModelDlg.h"

class TTaskDlg : public TModelDlg, private Ui::TTaskDlg
  {
    Q_OBJECT

  public:
    explicit TTaskDlg(const QSqlRecord &record, QWidget *parent = nullptr);
    QSqlRecord record() const;

  protected slots:
    void accept();

  private:
    bool m_task;

  };
