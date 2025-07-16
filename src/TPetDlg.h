#pragma once

#include "ui_TPetDlg.h"
#include "TModelDlg.h"

class TPetDlg : public TModelDlg, private Ui::TPetDlg
  {
    Q_OBJECT

  public:
    explicit TPetDlg(const QSqlRecord &record, QWidget *parent = nullptr);
    QSqlRecord record() const override;

  protected:

  protected slots:
    void accept();

    void speciesChanged(int index);
    void clientChanged(int index);

    void appendClient();
    void editClient();

    void vetPassportChanged(const QString &text);
    void generateVetPassport();

  signals:
    void updateDataNeeded(TSqlTableModel *model, quint32 id);
  };
