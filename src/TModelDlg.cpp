#include "TModelDlg.h"
#include "TSettings.h"

TModelDlg::TModelDlg(const QSqlRecord &record, const QString &configPath, QWidget *parent) : QDialog(parent), m_configPath(configPath)
  {
  m_id=record.value("id").toUInt();
  }

void TModelDlg::writeSettings()
  {
  TSettings().setXmlValue(m_configPath+"/geometry","",saveGeometry());
  }

void TModelDlg::readSettings()
  {
  restoreGeometry(TSettings().getXmlValue(m_configPath+"/geometry","",QByteArray()).toByteArray());
  }

void TModelDlg::accept()
  {
  writeSettings();
  QDialog::accept();
  }

void TModelDlg::reject()
  {
  writeSettings();
  QDialog::reject();
  }

quint32 TModelDlg::getId() const
  {
  return m_id;
  }

TDataModule *TModelDlg::dataModule() const
  {
  return TDataModule::instance();
  }
