#include "TDoctorDlg.h"

#include "TDataModule.h"
#include "TSettings.h"

#include "TProfilesModel.h"
#include "TDegreesModel.h"
#include "TRanksModel.h"
#include "TPetagesModel.h"
#include "TActionsModel.h"
#include "TSpeciesModel.h"

#include "TRoutinesModel.h"
#include "TRoutinesFilterModel.h"
#include "TRoutinesDelegate.h"

#include "TApptsModel.h"

#include "TDadataApiWorker.h"

#include "TPrefferedDlg.h"

#include <QMessageBox>
#include <QRegularExpressionValidator>

#include <QJsonObject>
#include <QJsonValue>
#include <QJsonArray>

#include <QTimer>
#include <QSqlQuery>

TDoctorDlg::TDoctorDlg(const QSqlRecord &record, QWidget *parent) : TModelDlg(record, "doctor_dialog", parent),
  m_suggestTimer(new QTimer(this)),
  m_availableChanged(false),
  m_intervalChanged(false),
  m_routinesChanged(false)

  {
  setupUi(this);

  QString name=record.value("name").toString();
  setWindowTitle(tr("Doctor [%1]").arg(name.isEmpty()==true ? tr("New doctor") : name));

  connect(prefferedBtn,&QPushButton::clicked,this,&TDoctorDlg::selectPreffered);
  connect(copyRoutineBtn,&QPushButton::clicked,this,&TDoctorDlg::copyRoutine);

  /* поля в диалоге: Имя, Телефон, Адрес и т.д. */
  nameEdit->setText(record.value("name").toString());

  TProfilesModel *profilesModel=qobject_cast<TProfilesModel*>(dataModule()->tableModel("profiles"));
  profileCombo->setModel(profilesModel,{"profile"},"id");
  profileCombo->setCurrentIndex(profileCombo->findData(record.value("profile_id")));

  birthdateEdit->setDate(record.value("birth_date").toDate());
  // про пол (Жен - 0, false; Муж - 1, true)
  genderCombo->addItem(tr("Female"),false);
  genderCombo->addItem(tr("Male"),true);
  genderCombo->setCurrentIndex(genderCombo->findData(record.value("gender").toBool()));

  phoneEdit->setText(record.value("phone_number").toString());
  telegramEdit->setText(record.value("telegram").toString());
  emailEdit->setText(record.value("email").toString());
  addressCombo->setCurrentText(record.value("address").toString());

  certEdit->setText(record.value("certificate").toString());
  certDateEdit->setDate(record.value("certificate_date").toDate());
  experienceSpin->setValue(record.value("experience").toUInt());

  TDegreesModel *degreesModel=qobject_cast<TDegreesModel*>(dataModule()->tableModel("degrees"));
  degreeCombo->setModel(degreesModel,{"degree"},"id");
  degreeCombo->setCurrentIndex(degreeCombo->findData(record.value("degree_id")));

  TRanksModel *ranksModel=qobject_cast<TRanksModel*>(dataModule()->tableModel("ranks"));
  rankCombo->setModel(ranksModel,{"rank"},"id");
  rankCombo->setCurrentIndex(rankCombo->findData(record.value("rank_id")));

  TPetagesModel *petagesModel=qobject_cast<TPetagesModel*>(dataModule()->tableModel("petages"));
  petageCombo->setModel(petagesModel,{"pet_age"},"id");
  petageCombo->setCurrentIndex(petageCombo->findData(record.value("petage_id")));

  TActionsModel *actionsModel=qobject_cast<TActionsModel*>(dataModule()->tableModel("actions"));
  actionCombo->setModel(actionsModel,{"action"},"id");
  actionCombo->setCurrentIndex(actionCombo->findData(record.value("action_id")));

  intervalSpin->setValue(record.value("interval").toUInt());
  connect(intervalSpin,qOverload<int>(&QSpinBox::valueChanged),this,[this](int value) {
    Q_UNUSED(value)
    m_intervalChanged=true;
    });

  prefferedEdit->setPlainText(record.value("preffered").toString());

  quint32 doctorId=getId();
  availableCheck->setChecked(doctorId>0 ? record.value("available").toBool()==true : true);
  connect(availableCheck,&QCheckBox::stateChanged,this,[this](int state) {
    Q_UNUSED(state)
    m_availableChanged=true;
    });

  // еще нету доктора (добавляется доктор, а id нужен) - узнать ожидаемый номер ID (автоинермент) доктора
  if (doctorId==0)
    doctorId=dataModule()->lastAutoInc("doctors")+1;

  // если и сейчас 0, то запрос вернул -1 - а это косяк с БД или именем таблицы - просто скажем (такого не должно быть, но ...)
  if (doctorId==0)
    QMessageBox::critical(this,tr("Error"),tr("Database not opened.\nCan't create doctor routine."),QMessageBox::Ok);

  /* Про расписание,  таблицы oddRoutineTable и evenRoutineTable (часы приема)*/
  m_routinesModel=qobject_cast<TRoutinesModel*>(dataModule()->tableModel("routines"));
  m_routinesModel->setFilter(QString("routines.doctor_id=%1").arg(doctorId));

  // зафиксировать, если поменяли расписание, в accept() сделать emit routinesChanged()
  connect(m_routinesModel,&TRoutinesModel::dataChanged,this,[this]() {
    m_routinesChanged=true;
    });

  // если нету достаточного количества записей графика работы для доктора - создаем
  if (m_routinesModel->rowCount()<14 && doctorId>0)
    {
    // создать 14 записей в таблице routines
    QSqlRecord record=dataModule()->tableRecord("routines");
    for (quint8 i=0;i<14;i++)
      {
      // проверить, нет ли такой записи уже
      QModelIndexList indexes=m_routinesModel->match(m_routinesModel->index(0,m_routinesModel->fieldIndex("day")),Qt::DisplayRole,i,1,Qt::MatchExactly);
      if (indexes.size()>0)
        continue;

      record.setValue("day",i);
      record.setValue("doctor_id",doctorId);
      record.setValue("from_am","09:00:00");
      record.setValue("from_pm","14:00:00");

      quint8 weekDay=i % 7;
      if (weekDay==5 || weekDay==6) // выходные
        {
        record.setValue("to_am","09:00:00");
        record.setValue("to_pm","14:00:00");
        }
      else // буднии
        {
        record.setValue("to_am","13:00:00");
        record.setValue("to_pm","18:00:00");
        }

      m_routinesModel->insertRecord(-1,record);
      }
    }

  /* фильтрующие модели, нечетная и четная недели */
  m_oddRoutineModel=new TRoutinesFilterModel(TRoutinesFilterModel::Odd, this);
  m_oddRoutineModel->setSourceModel(m_routinesModel);
  oddRoutineTable->setModel(m_oddRoutineModel);

  m_evenRoutineModel=new TRoutinesFilterModel(TRoutinesFilterModel::Even, this);
  m_evenRoutineModel->setSourceModel(m_routinesModel);
  evenRoutineTable->setModel(m_evenRoutineModel);

  /* делегат для отображения и редактирования графиков работы доктора */
  TRoutinesDelegate *routinesDelegate=new TRoutinesDelegate(this);
  oddRoutineTable->setItemDelegate(routinesDelegate);
  evenRoutineTable->setItemDelegate(routinesDelegate);

  /* запомнить последний адрес, для Dadata API */
  m_lastText=addressCombo->currentText();

  connect(phoneEdit,&QLineEdit::cursorPositionChanged,this,&TDoctorDlg::phonePostionChenged);

  okBtn->setShortcut(QKeySequence(Qt::Key_Enter));
  cancelBtn->setShortcut(QKeySequence(Qt::Key_Cancel));

  connect(okBtn,&QPushButton::clicked,this,&TDoctorDlg::accept);
  connect(cancelBtn,&QPushButton::clicked,this,&TDoctorDlg::reject);

  m_suggestTimer->setTimerType(Qt::PreciseTimer);
  m_suggestTimer->setInterval(1000);
  m_suggestTimer->setSingleShot(true);
  connect(m_suggestTimer,&QTimer::timeout,this,&TDoctorDlg::showSuggest);

  addressCombo->setReplaceKeyEvent(true);
  // по нажатию кнопки запускаем таймер (1 сек), быстрое нажатие - перезапустит
  connect(addressCombo,&TComboBox::keyPressed,this,[this]() {
    m_suggestTimer->start();
    });

  // выбор текста из меню сделат последний текст равным выбранному
  connect(addressCombo,qOverload<int>(&TComboBox::activated),this,[this](int index) {
    m_lastText=addressCombo->itemText(index);
    });

  telegramEdit->setValidator(new QIntValidator(telegramEdit));

  nameEdit->setFocus();
  readSettings();
  }

QSqlRecord TDoctorDlg::record() const
  {
  QSqlRecord record=dataModule()->tableRecord("doctors");

  record.setValue("name", nameEdit->text());
  record.setValue("birth_date", birthdateEdit->date().toString("yyyy-MM-dd"));
  record.setValue("gender", genderCombo->currentData());
  record.setValue("phone_number", phoneEdit->text());
  record.setValue("email", emailEdit->text());
  record.setValue("telegram", telegramEdit->text());
  record.setValue("address", addressCombo->currentText());
  record.setValue("certificate", certEdit->text());
  record.setValue("certificate_date", certDateEdit->date().toString("yyyy-MM-dd"));
  record.setValue("experience", experienceSpin->value());

  record.setValue("profile_id", profileCombo->currentData());
  record.setValue("degree_id", degreeCombo->currentData());
  record.setValue("rank_id", rankCombo->currentData());
  record.setValue("petage_id", petageCombo->currentData());
  record.setValue("action_id", actionCombo->currentData());

  record.setValue("interval", intervalSpin->value());

  record.setValue("preffered",prefferedEdit->toPlainText());
  record.setValue("available",availableCheck->checkState()==Qt::Checked);

  return record;
  }

void TDoctorDlg::writeSettings()
  {
  TSettings settings;
  settings.setXmlValue("doctor_dialog/splitter","",doctorSplitter->saveState());
  settings.setXmlValue("doctor_dialog/odd_routine_table","",oddRoutineTable->horizontalHeader()->saveState());
  settings.setXmlValue("doctor_dialog/even_routine_table","",evenRoutineTable->horizontalHeader()->saveState());

  TModelDlg::writeSettings();
  }

void TDoctorDlg::readSettings()
  {
  TModelDlg::readSettings();

  TSettings settings;
  doctorSplitter->setSizes({width()/3, 2*width()/3});
  doctorSplitter->restoreState(settings.getXmlValue("doctor_dialog/splitter","",QByteArray()).toByteArray());
  oddRoutineTable->horizontalHeader()->restoreState(settings.getXmlValue("doctor_dialog/odd_routine_table","",QByteArray()).toByteArray());
  evenRoutineTable->horizontalHeader()->restoreState(settings.getXmlValue("doctor_dialog/even_routine_table","",QByteArray()).toByteArray());

  // сказать, какие поля скрыть в таблицах
  QStringList hiddenFields={"id", "doctor_id", "name", "phone_number", "telegram", "interval", "available", "profile_id", "profile"};
  QSqlRecord record=m_routinesModel->record();
  for (quint8 i=0; i<record.count(); i++)
    {
    oddRoutineTable->setColumnHidden(i, hiddenFields.contains(record.fieldName(i)));
    evenRoutineTable->setColumnHidden(i, hiddenFields.contains(record.fieldName(i)));
    }

  oddRoutineTable->verticalHeader()->setDefaultSectionSize(qApp->font().pointSize()*2.5);
  evenRoutineTable->verticalHeader()->setDefaultSectionSize(qApp->font().pointSize()*2.5);
  }

void TDoctorDlg::checkShedule()
  {
  // если ничего не поменялось касаемо расписания, доступности, интервала приема, то уходим
  if (m_availableChanged==false && m_intervalChanged==false && m_routinesChanged==false)
    return;

  // во-первых, надо надо уведомить об изенении расписания, если оно было и применить изменения в routines
  if (m_routinesChanged==true)
    {
    m_routinesModel->submitAll();
    emit routinesChanged();
    }

  // во-вторых, узнать, есть ли у доктора "не протухшие" (сегодня или позже) записи на прием
  QSqlQuery query(dataModule()->database());
  query.prepare(QString("select appt_date from appts where doctor_id=%1 and state=%2 and appt_date>='%3' group by appt_date").
                arg(QString::number(getId()), QString::number((quint8)TApptsModel::Created), QDate::currentDate().toString("yyyy-MM-dd")));

  QStringList dates;
  query.exec();
  while (query.next()==true)
    dates << query.value("appt_date").toDate().toString("dd.MM.yyyy");

  if (dates.size()>0)
    {
    QString text=tr("The doctor appointment hours, availibility\nor appointment interval has been changed.\n"
                    "Don't forget to check and change\nappointments on the 'Shedule' tab.");

    QMessageBox messageBox(QMessageBox::Warning, tr("Attention"), text, QMessageBox::Ok);
    messageBox.setInformativeText(tr("Press 'Show details' to see the appointment dates."));
    messageBox.setDetailedText(dates.join(", "));

    QCheckBox *checkBox=new QCheckBox(tr("Display appointments on dialog close."),&messageBox);
    messageBox.setCheckBox(checkBox);
    messageBox.exec();

    QMap <QDate, QList<quint32>> expanded;
    if (messageBox.checkBox()->isChecked()==true)
      {
      foreach (const QString date,dates)
        expanded.insert(QDate::fromString(date,"dd.MM.yyyy"),{getId()});

      QVariantList path={getId()};
      if (dates.size()>0)
        path.prepend(QDate::fromString(dates.first(),"dd.MM.yyyy"));

      emit showApptsNeeded(expanded);
      }
    }
  }

void TDoctorDlg::phonePostionChenged(int oldPos, int newPos)
  {
  Q_UNUSED(oldPos)
  Q_UNUSED(newPos)

  quint16 position=phoneEdit->cursorPosition();
  quint16 size=phoneEdit->text().size();
  if (position>size)
    phoneEdit->setCursorPosition(size);
  }

void TDoctorDlg::selectPreffered()
  {
  TPrefferedDlg prefferedDlg(prefferedEdit->toPlainText(), this);
  if (prefferedDlg.exec()!=QDialog::Accepted)
    return;

  prefferedEdit->setPlainText(prefferedDlg.preffered());
  }

void TDoctorDlg::copyRoutine()
  {
  for (quint8 i=0;i<m_oddRoutineModel->rowCount();i++)
    {
    QVariant fromAm=m_oddRoutineModel->index(i,m_routinesModel->fieldIndex("from_am")).data();
    QVariant toAm=m_oddRoutineModel->index(i,m_routinesModel->fieldIndex("to_am")).data();
    QVariant fromPm=m_oddRoutineModel->index(i,m_routinesModel->fieldIndex("from_pm")).data();
    QVariant toPm=m_oddRoutineModel->index(i,m_routinesModel->fieldIndex("to_pm")).data();

    if (m_evenRoutineModel->rowCount()<i+1) // не должно быть такого, но так, на всякий
        return;

    m_evenRoutineModel->setData(m_evenRoutineModel->index(i,m_routinesModel->fieldIndex("from_am")),fromAm);
    m_evenRoutineModel->setData(m_evenRoutineModel->index(i,m_routinesModel->fieldIndex("to_am")),toAm);
    m_evenRoutineModel->setData(m_evenRoutineModel->index(i,m_routinesModel->fieldIndex("from_pm")),fromPm);
    m_evenRoutineModel->setData(m_evenRoutineModel->index(i,m_routinesModel->fieldIndex("to_pm")),toPm);
    }
  }

void TDoctorDlg::accept()
  {
  if (nameEdit->text().isEmpty()==true)
    {
    QMessageBox::critical(this,tr("Error"),tr("Name can't be empty."),QMessageBox::Ok);
    nameEdit->setFocus();
    return;
    }

  if (birthdateEdit->date().isValid()==false)
    {
    QMessageBox::critical(this,tr("Error"),tr("Invalid birthdate."),QMessageBox::Ok);
    birthdateEdit->setFocus();
    return;
    }

  if (genderCombo->currentIndex()<0)
    {
    QMessageBox::critical(this,tr("Error"),tr("Gender can't be empty."),QMessageBox::Ok);
    genderCombo->setFocus();
    return;
    }

  QRegularExpressionValidator phoneValidator(QRegularExpression("^\\+7[0-9]{10}$", QRegularExpression::CaseInsensitiveOption));
  phoneEdit->setValidator(&phoneValidator);
  if (phoneEdit->hasAcceptableInput()==false)
    {
    QMessageBox::critical(this,tr("Error"),tr("Invalid Phone number."),QMessageBox::Ok);
    phoneEdit->setFocus();
    return;
    }

  QRegularExpressionValidator emailValidator(QRegularExpression("^[A-Za-z0-9._%+-]+@[A-Za-z0-9.-]+\\.[A-Za-z]{2,}$")); // email address
  emailEdit->setValidator(&emailValidator);
  if (emailEdit->hasAcceptableInput()==false)
    {
    QMessageBox::critical(this,tr("Error"),tr("Invalid email address."),QMessageBox::Ok);
    emailEdit->setFocus();
    return;
    }

  if (certEdit->text().isEmpty()==true)
    {
    QMessageBox::critical(this,tr("Error"),tr("Certificate can't be empty."),QMessageBox::Ok);
    certEdit->setFocus();
    return;
    }

  if (certDateEdit->date().isValid()==false)
    {
    QMessageBox::critical(this,tr("Error"),tr("Invalid certificate date."),QMessageBox::Ok);
    certDateEdit->setFocus();
    return;
    }

  if (profileCombo->currentIndex()<0)
    {
    QMessageBox::critical(this,tr("Error"),tr("Profile can't be empty."),QMessageBox::Ok);
    certDateEdit->setFocus();
    return;
    }

  TDadataApiWorker *dadataApiWorker=TDadataApiWorker::instance();
  if (dadataApiWorker==nullptr)
    {
    TModelDlg::accept();
    return;
    }

  // проверим адрес
  m_lastText=addressCombo->currentText();
  if (m_lastText.isEmpty()==true)
    {
    QMessageBox::critical(this,tr("Error"),tr("Address can't be empty."),QMessageBox::Ok);
    addressCombo->setFocus();
    return;
    }

  QJsonObject response=dadataApiWorker->sendApiRequest("Address",{{"query",m_lastText},{"count",20}});

  qint16 code=response.value("code").toInt();
  quint16 errorCode=response.value("error_code").toInt();
  //QString errorText=response.value("error_string").toString();

  // ошибка, не считаем ее критической, не смогльи проверить адрес, ну и ладно...
  if (errorCode!=QNetworkReply::NoError || code!=200)
    {
    checkShedule();
    TModelDlg::accept();
    return;
    }

  QJsonArray array=response.value("suggestions").toArray();

  // если в полученных ответах нет точного совпадения - то косяк
  for (const QJsonValue &value: array)
    {
    QJsonObject object=value.toObject();
    if (m_lastText!=object.value("value").toString()) // не попали точно
      continue;

    checkShedule();
    TModelDlg::accept();
    return;
    }

  // если тут, то не нашлось адреса точно совпадающего с введнным
  QMessageBox::critical(this,tr("Error"),tr("Address seems not valid.\nFix it, please."),QMessageBox::Ok);
  addressCombo->setFocus();
  }

void TDoctorDlg::reject()
  {
  m_routinesModel->revertAll();
  TModelDlg::reject();
  }

void TDoctorDlg::showSuggest()
  {
  // набирает текст, не мешать или просто пустота
  if (m_lastText==addressCombo->currentText() || addressCombo->currentText().isEmpty()==true)
    return;

  TDadataApiWorker *dadataApiWorker=TDadataApiWorker::instance();
  if (dadataApiWorker==nullptr)
    return;

  m_lastText=addressCombo->currentText();
  QJsonObject response=dadataApiWorker->sendApiRequest("Address",{{"query",m_lastText},{"count",20}});

  qint16 code=response.value("code").toInt();
  quint16 errorCode=response.value("error_code").toInt();
  //QString errorText=response.value("error_string").toString();

  // ошибка, просто уходим
  if (errorCode!=QNetworkReply::NoError || code!=200)
    return;

  QJsonArray array=response.value("suggestions").toArray();
  // пустой ответ, просто уходим
  if (array.isEmpty()==true)
    return;

  QString text=addressCombo->currentText();
  addressCombo->clear();
  for (const QJsonValue &value: array)
    {
    QJsonObject object=value.toObject();
    addressCombo->addItem(object.value("value").toString(), object.value("unrestricted_value").toString());
    }

  addressCombo->setCurrentText(text);
  addressCombo->showPopup();
  }
