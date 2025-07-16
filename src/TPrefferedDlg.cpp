#include "TPrefferedDlg.h"

#include "TSettings.h"
#include "TDataModule.h"
#include "TSpeciesModel.h"

TPrefferedDlg::TPrefferedDlg(const QString &preffered, QWidget *parent) : QDialog(parent)
  {
  setupUi(this);

  connect(okBtn,&QPushButton::clicked,this,&TPrefferedDlg::accept);
  connect(cancelBtn,&QPushButton::clicked,this,&TPrefferedDlg::reject);

  connect(speciesList,&QListWidget::itemClicked,this,[this](QListWidgetItem *item) {
    item->setCheckState(item->checkState()==Qt::Checked ? Qt::Unchecked : Qt::Checked);
    });

  TSpeciesModel *speciesModel=qobject_cast<TSpeciesModel*>(TDataModule::instance()->tableModel("species"));
  QStringList selected=preffered.split(";");

  for (quint16 i=0;i<speciesModel->rowCount();i++)
    {
    QSqlRecord record=speciesModel->record(i);
    QString species=record.value("species").toString();

    QListWidgetItem *item=new QListWidgetItem(species);
    item->setCheckState(selected.contains(species) ? Qt::Checked : Qt::Unchecked);
    speciesList->addItem(item);
    }

  readSettings();
  }

QString TPrefferedDlg::preffered() const
  {
  QStringList preffered;
  for (quint16 i=0;i<speciesList->count();i++)
    {
    QListWidgetItem *item=speciesList->item(i);
    if (item->checkState()==Qt::Checked)
      preffered << item->text();
    }

  return preffered.join(";");
  }

void TPrefferedDlg::writeSettings()
  {
  TSettings().setXmlValue("preffered_dialog/geometry","",saveGeometry());
  }

void TPrefferedDlg::readSettings()
  {
  restoreGeometry(TSettings().getXmlValue("preffered_dialog/geometry","",QByteArray()).toByteArray());
  }

void TPrefferedDlg::accept()
  {
  writeSettings();
  QDialog::accept();
  }

void TPrefferedDlg::reject()
  {
  writeSettings();
  QDialog::reject();
  }
