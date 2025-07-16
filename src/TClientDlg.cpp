#include "TClientDlg.h"
#include "TDataModule.h"

#include "TDadataApiWorker.h"

#include <QMessageBox>
#include <QRegularExpressionValidator>

#include <QJsonObject>
#include <QJsonValue>
#include <QJsonArray>

#include <QTimer>

TClientDlg::TClientDlg(const QSqlRecord &record, QWidget *parent) : TModelDlg(record, "client_dialog", parent),
  m_suggestTimer(new QTimer(this))
  {
  setupUi(this);

  QString name=record.value("name").toString();
  setWindowTitle(tr("Client [%1]").arg(name.isEmpty()==true ? tr("New client") : name));

  /* поля в диалоге: Имя, Телефон, Telegram, Паспорт, Адрес */
  nameEdit->setText(record.value("name").toString());
  phoneEdit->setText(record.value("phone_number").toString());
  telegramEdit->setText(record.value("telegram").toString());
  passportEdit->setText(record.value("passport").toString());
  addressCombo->setCurrentText(record.value("address").toString());

  m_lastText=addressCombo->currentText();

  connect(phoneEdit,&QLineEdit::cursorPositionChanged,this,&TClientDlg::phonePostionChenged);
  connect(passportEdit,&QLineEdit::cursorPositionChanged,this,&TClientDlg::passportPostionChenged);

  okBtn->setShortcut(QKeySequence(Qt::Key_Enter));
  cancelBtn->setShortcut(QKeySequence(Qt::Key_Cancel));

  connect(okBtn,&QPushButton::clicked,this,&TClientDlg::accept);
  connect(cancelBtn,&QPushButton::clicked,this,&TClientDlg::reject);

  m_suggestTimer->setTimerType(Qt::PreciseTimer);
  m_suggestTimer->setInterval(1000);
  m_suggestTimer->setSingleShot(true);
  connect(m_suggestTimer,&QTimer::timeout,this,&TClientDlg::showSuggest);

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

QSqlRecord TClientDlg::record() const
  {
  QSqlRecord record=dataModule()->tableRecord("clients");

  record.setValue("name", nameEdit->text());
  record.setValue("phone_number", phoneEdit->text());
  record.setValue("passport", passportEdit->text());
  record.setValue("address", addressCombo->currentText());
  record.setValue("telegram", telegramEdit->text());

  return record;
  }

void TClientDlg::phonePostionChenged(int oldPos, int newPos)
  {
  Q_UNUSED(oldPos)
  Q_UNUSED(newPos)

  quint16 position=phoneEdit->cursorPosition();
  quint16 size=phoneEdit->text().size();
  if (position>size)
    phoneEdit->setCursorPosition(size);
  }

void TClientDlg::passportPostionChenged(int oldPos, int newPos)
  {
  Q_UNUSED(oldPos)
  Q_UNUSED(newPos)

  quint16 position=passportEdit->cursorPosition();
  quint16 size=passportEdit->text().size();
  qint16 space=passportEdit->inputMask().indexOf(" ");

  if (position>size)
    passportEdit->setCursorPosition(size>space ? size : size-1); // а size==space быть не может :o)
  }

void TClientDlg::accept()
  {
  if (nameEdit->text().isEmpty()==true)
    {
    QMessageBox::critical(this,tr("Error"),tr("It is needed to specify the client name."),QMessageBox::Ok);
    nameEdit->setFocus();
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

  QRegularExpressionValidator passportValidator(QRegularExpression("^[0-9]{4}\\s[0-9]{6}$", QRegularExpression::CaseInsensitiveOption));
  passportEdit->setValidator(&passportValidator);
  if (passportEdit->hasAcceptableInput()==false)
    {
    QMessageBox::critical(this,tr("Error"),tr("Invalid Passport."),QMessageBox::Ok);
    passportEdit->setFocus();
    return;
    }

  TDadataApiWorker *dadataApiWorker=TDadataApiWorker::instance();
  if (dadataApiWorker==nullptr)
    {
    TModelDlg::accept();
    return;
    }

  // FIXME: проверить паспорт через datdataAPI
  /*QJsonObject response=dadataApiWorker->sendApiRequest("Passport",{{"",passportEdit->text()}});
  qDebug()<<response;
  //m_suggestTimer->stop();

  qint16 code=response.value("code").toInt();
  quint16 errorCode=response.value("error_code").toInt();
  //QString errorText=response.value("error_string").toString();

  // ошибки нет, но ответ != паспорт валидный
  if (errorCode==QNetworkReply::NoError && code==200 )
    {

    }*/

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

  // ошибка, просто уходим
  if (errorCode!=QNetworkReply::NoError || code!=200)
    {
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

    // а если оказались тут, то попали точно
    TModelDlg::accept();
    return;
    }

  // если тут, то не нашлось адреса точно совпадающего с введнным
  QMessageBox::critical(this,tr("Error"),tr("Address seems not valid.\nFix it, please."),QMessageBox::Ok);
  addressCombo->setFocus();
  }

void TClientDlg::showSuggest()
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
  /*addressCombo->lineEdit()->setFocus();
  addressCombo->lineEdit()->setReadOnly(false);*/
  }
