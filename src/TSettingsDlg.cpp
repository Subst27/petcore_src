#include "TSettingsDlg.h"

#include "TSettings.h"

#include <QFontDialog>

TSettingsDlg::TSettingsDlg(QWidget *parent) : QDialog(parent)
  {
  setupUi(this);

  okBtn->setShortcut(QKeySequence(Qt::Key_Enter));
  cancelBtn->setShortcut(QKeySequence(Qt::Key_Cancel));

  connect(okBtn,&QPushButton::clicked,this,&TSettingsDlg::accept);
  connect(cancelBtn,&QPushButton::clicked,this,&TSettingsDlg::reject);

  connect(fontBtn,&QPushButton::clicked,this,&TSettingsDlg::selectFont);

  connect(archSizeCheck,&QCheckBox::stateChanged,this,[this](int state) {
    archSizeSpin->setEnabled(state==Qt::Checked);
    });

  connect(autoArchCheck,&QCheckBox::stateChanged,this,[this](int state) {
    archPeriodSpin->setEnabled(state==Qt::Checked);
    });

  readSettings();
  }

void TSettingsDlg::writeSettings()
  {
  TSettings().setXmlValue("settings_dialog/geometry","",saveGeometry());
  }

void TSettingsDlg::readSettings()
  {
  TSettings settings;
  restoreGeometry(settings.getXmlValue("settings_dialog/geometry","",QByteArray()).toByteArray());

  fontBtn->setFont(settings.getXmlValue("main_window/font","",qApp->font()).value<QFont>());
  remDeadlineSpin->setValue(settings.getXmlValue("tasks_dock/reminder_deadline","",3).toUInt());
  // настройки архивации
  bool alarm=settings.getXmlValue("archiving/enable_alarm","",false).toBool();
  archSizeCheck->setChecked(alarm);
  archSizeSpin->setEnabled(alarm);
  archSizeSpin->setValue(settings.getXmlValue("archiving/alarm_size","",30).toUInt());

  bool autoArch=settings.getXmlValue("archiving/enable_auto","",false).toBool();
  autoArchCheck->setChecked(autoArch);
  archPeriodSpin->setEnabled(autoArch);
  archPeriodSpin->setValue(settings.getXmlValue("archiving/period","",30).toUInt());

  archPastSpin->setValue(settings.getXmlValue("archiving/keep_past","",99).toUInt());
  }

void TSettingsDlg::accept()
  {
  TSettings settings;
  settings.setXmlValue("main_window/font","",fontBtn->font());
  settings.setXmlValue("tasks_dock/reminder_deadline","",remDeadlineSpin->value());

  // записать настройки архивации в конфиг
  settings.setXmlValue("archiving/enable_alarm","",archSizeCheck->isChecked());
  settings.setXmlValue("archiving/alarm_size","",archSizeSpin->value());
  settings.setXmlValue("archiving/enable_auto","",autoArchCheck->isChecked());
  settings.setXmlValue("archiving/period","",archPeriodSpin->value());
  settings.setXmlValue("archiving/keep_past","",archPastSpin->value());

  writeSettings();
  QDialog::accept();
  }

void TSettingsDlg::reject()
  {
  writeSettings();
  QDialog::reject();
  }

void TSettingsDlg::selectFont()
  {
  QFont font=fontBtn->font();
  bool ok;
  font=QFontDialog::getFont(&ok,font,this,tr("Application font"));
  if (ok==false)
    return;

  fontBtn->setFont(font);
  }
