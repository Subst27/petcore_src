#pragma once

#include "ui_TApptDlg.h"
#include "TModelDlg.h"

#include "TMainWnd.h"

class TApptDlg : public TModelDlg, private Ui::TApptDlg
  {
    Q_OBJECT

  public:
    explicit TApptDlg(TMainWnd::PageTabs pageTab, const QSqlRecord &record, QWidget *parent = nullptr);
    QSqlRecord record() const override;

  protected:

  protected slots:
    void accept();

    void doctorChanged(int index);
    void clientChanged(int index);
    void petChanged(int index);

    void appendDoctor();
    void editDoctor();

    void appendClient();
    void editClient();

    void appendPet();
    void editPet();

    void solveTimeSlots();

  private:
    TMainWnd::PageTabs m_pageTab;

  signals:
    void updateDataNeeded(TSqlTableModel *model, quint32 id);
  };
