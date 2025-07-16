#pragma once

#include "ui_TClientDlg.h"
#include "TModelDlg.h"

class TClientDlg : public TModelDlg, private Ui::TClientDlg
  {
    Q_OBJECT

  public:
    explicit TClientDlg(const QSqlRecord &record, QWidget *parent = nullptr);
    QSqlRecord record() const;

  protected slots:
    void phonePostionChenged(int oldPos,int newPos);
    void passportPostionChenged(int oldPos,int newPos);

    void accept();

    void showSuggest();

  private:
    QString m_lastText;
    QTimer *m_suggestTimer;
  };
