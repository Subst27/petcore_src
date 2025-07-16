#include "TPetDlg.h"
#include "TDataModule.h"

#include "TClientsModel.h"
#include "TSpeciesModel.h"
#include "TBreedsModel.h"
#include "TStatusesModel.h"
#include "TMarkingModel.h"

#include "TSettings.h"

#include <QMessageBox>

TPetDlg::TPetDlg(const QSqlRecord &record, QWidget *parent) : TModelDlg(record, "pet_dialog", parent)
  {
  setupUi(this);

  QString name=record.value("name").toString();
  setWindowTitle(tr("Pet [%1]").arg(name.isEmpty()==true ? tr("New pet") : name));

  /* поля в диалоге... */
  nameEdit->setText(record.value("name").toString());

  TSpeciesModel *speciesModel=qobject_cast<TSpeciesModel*>(dataModule()->tableModel("species"));
  speciesCombo->setModel(speciesModel,{"species"},"id");
  connect(speciesCombo,qOverload<int>(&QComboBox::currentIndexChanged),this,&TPetDlg::speciesChanged);
  speciesCombo->setCurrentIndex(speciesCombo->findData(record.value("species_id")));

  breedCombo->setCurrentIndex(breedCombo->findData(record.value("breed_id")));
  birthdateEdit->setDate(record.value("birth_date").toDate());

  TMarkingModel *markingModel=qobject_cast<TMarkingModel*>(dataModule()->tableModel("marking"));
  markingCombo->setModel(markingModel,{"marking"},"id");
  markingCombo->setCurrentIndex(markingCombo->findData(record.value("marking_id")));

  markingDateEdit->setDate(record.value("marking_date").toDate());
  uicmmEdit->setText(record.value("uicmm").toString());

  connect(vetPassportEdit,&QLineEdit::textChanged,this,&TPetDlg::vetPassportChanged);
  connect(generateBtn,&QPushButton::clicked,this,&TPetDlg::generateVetPassport);
  vetPassportEdit->setText(record.value("vet_passport").toString());

  TStatusesModel *statusesModel=qobject_cast<TStatusesModel*>(dataModule()->tableModel("statuses"));
  statusCombo->setModel(statusesModel,{"status"},"id");
  statusCombo->setCurrentIndex(statusCombo->findData(record.value("status_id")));

  TClientsModel *clientsModel=qobject_cast<TClientsModel*>(dataModule()->tableModel("clients"));
  clientCombo->setModel(clientsModel,{"name","phone_number","telegram"},"id");
  connect(clientCombo,qOverload<int>(&TComboBox::currentIndexChanged),this,&TPetDlg::clientChanged);
  clientCombo->setCurrentIndex(clientCombo->findData(record.value("client_id")));

  editClientBtn->setEnabled(clientCombo->currentIndex()>-1);

  connect(appendClientBtn,&QPushButton::clicked,this,&TPetDlg::appendClient);
  connect(editClientBtn,&QPushButton::clicked,this,&TPetDlg::editClient);

  okBtn->setShortcut(QKeySequence(Qt::Key_Enter));
  cancelBtn->setShortcut(QKeySequence(Qt::Key_Cancel));

  connect(okBtn,&QPushButton::clicked,this,&TPetDlg::accept);
  connect(cancelBtn,&QPushButton::clicked,this,&TPetDlg::reject);

  nameEdit->setFocus();
  readSettings();
  }

QSqlRecord TPetDlg::record() const
  {
  QSqlRecord record=dataModule()->tableRecord("pets");

  record.setValue("name", nameEdit->text());
  record.setValue("birth_date", birthdateEdit->date().toString("yyyy-MM-dd"));
  record.setValue("species_id", speciesCombo->currentData().toInt());
  record.setValue("breed_id", breedCombo->currentData().toInt());
  record.setValue("marking_id",markingCombo->currentData().toInt());
  record.setValue("marking_date",markingDateEdit->date().toString("yyyy-MM-dd"));
  record.setValue("uicmm", uicmmEdit->text());
  record.setValue("vet_passport", vetPassportEdit->text());
  record.setValue("client_id", clientCombo->currentData().toInt());
  record.setValue("status_id", statusCombo->currentData().toInt());

  return record;
  }

void TPetDlg::accept()
  {
  if (nameEdit->text().isEmpty()==true)
    {
    QMessageBox::critical(this,tr("Error"),tr("It is needed to specify the pet name."),QMessageBox::Ok);
    nameEdit->setFocus();
    return;
    }

  if (speciesCombo->currentData().toInt()<1)
    {
    QMessageBox::critical(this,tr("Error"),tr("It is needed to specify the pet species."),QMessageBox::Ok);
    nameEdit->setFocus();
    return;
    }

  if (uicmmEdit->text().isEmpty()==true)
    {
    QMessageBox::critical(this,tr("Error"),tr("It is needed to specify the pet UICMM."),QMessageBox::Ok);
    uicmmEdit->setFocus();
    return;
    }

  if (vetPassportEdit->text().isEmpty()==true)
    {
    QMessageBox::critical(this,tr("Error"),tr("It is needed to specify the vet passport."),QMessageBox::Ok);
    vetPassportEdit->setFocus();
    return;
    }

  if (clientCombo->currentData().toInt()<1)
    {
    QMessageBox::critical(this,tr("Error"),tr("It is needed to specify the client."),QMessageBox::Ok);
    clientCombo->setFocus();
    return;
    }

  if (statusCombo->currentData().toInt()<1)
    {
    QMessageBox::critical(this,tr("Error"),tr("It is needed to specify the status."),QMessageBox::Ok);
    statusCombo->setFocus();
    return;
    }

  TModelDlg::accept();
  }

void TPetDlg::speciesChanged(int index)
  {
  QString filter;
  if (speciesCombo->currentData().toInt()>0)
    filter="breeds.species_id="+speciesCombo->currentData().toString();

  TBreedsModel *breedsModel=qobject_cast<TBreedsModel*>(dataModule()->tableModel("breeds"));
  breedCombo->setModel(breedsModel,{"breed"},"id",filter);
  }

void TPetDlg::clientChanged(int index)
  {
  QMap<QString,QVariant> data=clientCombo->data(index);

  phoneEdit->setText(data.value("phone_number").toString());
  telegramEdit->setText(data.value("telegram").toString());

  editClientBtn->setEnabled(clientCombo->currentIndex()>-1);

  editClientBtn->setEnabled(index>-1);
  }

void TPetDlg::appendClient()
  {
  // запомнить текущий индекс в clientCombo
  qint32 index=clientCombo->currentIndex();
  // запомнить seq до вызова диалога добавления
  qint32 before=dataModule()->lastAutoInc("clients");

  TClientsModel *clientsModel=qobject_cast<TClientsModel*>(dataModule()->tableModel("clients"));
  emit updateDataNeeded(clientsModel, 0);

  // обновить модель в clientCombo и установить последнюю созданную запись
  clientCombo->setModel(clientsModel,{"name","phone_number","telegram"},"id");

  qint32 after=dataModule()->lastAutoInc("clients");
  if (after!=before)
    index=clientCombo->findData(after);

  clientCombo->setCurrentIndex(index);
  }

void TPetDlg::editClient()
  {
  // запомнить текущий индекс clientCombo
  qint32 index=clientCombo->currentIndex();

  TClientsModel *clientsModel=qobject_cast<TClientsModel*>(dataModule()->tableModel("clients"));
  emit updateDataNeeded(clientsModel, clientCombo->currentData().toUInt());

  // обновить модель в clientCombo и установить текущую запись
  clientCombo->setModel(clientsModel,{"name","phone_number","telegram"},"id");
  clientCombo->setCurrentIndex(index);
  }

void TPetDlg::vetPassportChanged(const QString &text)
  {
  generateBtn->setEnabled(text.isEmpty());
  }

void TPetDlg::generateVetPassport()
  {
  // формат ПЭТХХХXXХ-YYYYYY-DDMMYY, где XXXXXX - номер клиники с лидирующими нулями, YYYYYY - шесть букв, де факто число
  QString clinicId=TSettings().getXmlValue("clinic/id","","").toString();
  if (clinicId.isEmpty()==true)
    {
    QMessageBox::critical(this,tr("Error"),tr("The clinic ID not found.\nCan't generate vet passport."),QMessageBox::Ok);
    return;
    }

  // допинать нулями с начала до 6-ти символов
  while (clinicId.size()<6)
    clinicId.prepend("0");

  //QString letters={"АБВГДЕЖЗИКЛМНОПРСТУФХЦЧШЫЭЮЯ"}; // 28 символов - значит 28-чная система
  QString letters={"ABCEHKMNOPTY"}; // "ABCEHKMNOPTY" - символы, схожие по написанию в латинице и кириллице, 12-чная система счисления получится
  quint32 petId=getId();
  if (petId==0)
    {
    // узнать ожидаемый номер ID (автоинермент) животного
    petId=dataModule()->lastAutoInc("pets")+1;
    // если и сейчас petId==0, то запрос вернул -1 - что-то не так, такого не должно быть, но ...
    if (petId)
      {
      QMessageBox::critical(this,tr("Error"),tr("Database not opened.\nCan't generate vet passport."),QMessageBox::Ok);
      return;
      }
    }

  // перевести petId в строку, число разрядности кол-ва символов в letters
  QString petString;
  quint16 remainder;
  while (petId>0)
    {
    remainder = petId % letters.size();
    petId = petId / letters.size();
    petString.prepend(letters.at(remainder));
    }

  while (petString.size()<6)
    petString.prepend(letters.at(0));

  // текущую дату в нужном нам формате
  QString date=QDate::currentDate().toString("ddMMyy");
  vetPassportEdit->setText(tr("PET")+clinicId+"-"+petString+"-"+date);
  }
