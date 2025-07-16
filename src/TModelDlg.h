#pragma once

#include <QDialog>
#include <QSqlRecord>

#include "TDataModule.h"

class TModelDlg : public QDialog
  {
    Q_OBJECT
  public:
    TModelDlg(const QSqlRecord &record, const QString &configPath, QWidget *parent=nullptr);
    virtual QSqlRecord record() const = 0;

  protected:
    virtual void writeSettings();
    virtual void readSettings();

  protected slots:
    void accept();
    void reject();

  protected:
    quint32 getId() const;
    TDataModule *dataModule() const; // для удобства доступа просто, не более

  private:
    QString m_configPath;
    quint32 m_id;
  };

