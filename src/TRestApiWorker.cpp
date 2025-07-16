#include "TRestApiWorker.h"
#include "TSettings.h"
#include "QAesEncryption.h"

#include <QThread>
#include <QFileInfo>

#include <QNetworkAccessManager>

#include <QUrlQuery>
#include <QJsonDocument>
#include <QJsonArray>

TRestApiWorker::TRestApiWorker(QObject *parent)  : QObject{parent}
  {
  if (QMetaType::type("TRestApiWorker::HttpMethods")==QMetaType::UnknownType)
    qRegisterMetaType<TRestApiWorker::HttpMethods>("TRestApiWorker::HttpMethods");

  m_manager=new QNetworkAccessManager(this);
  m_manager->setRedirectPolicy(QNetworkRequest::NoLessSafeRedirectPolicy);
  m_manager->setAutoDeleteReplies(false);

  connect(m_manager,qOverload<QNetworkReply*,const QList<QSslError>&>(&QNetworkAccessManager::sslErrors),this,[this](QNetworkReply *reply,const QList<QSslError>) {
    reply->ignoreSslErrors();
    });
  }

void TRestApiWorker::initialize(const QString &path)
  {
  m_path=path;

  TSettings settings;
  m_baseUrl=settings.getXmlValue(m_path+"/server_url","","").toString();
  m_manager->setTransferTimeout(1000*settings.getXmlValue(m_path+"/request_timeout","",30).toUInt());

  QByteArray message=QByteArrayLiteral("Web_Success_Petcore");
  message=QCryptographicHash::hash(message, QCryptographicHash::Sha256);

  m_token=settings.getXmlValue(m_path+"/access_token","",QByteArray()).toByteArray();
  m_token=QByteArray::fromHex(m_token);
  m_token=QAESEncryption::Decrypt(QAESEncryption::AES_256, QAESEncryption::ECB, m_token, message);
  m_token=QAESEncryption::RemovePadding(m_token, QAESEncryption::ISO); //qDebug()<<m_token;

  m_secret=settings.getXmlValue(m_path+"/secret_key","",QByteArray()).toByteArray();
  m_secret=QByteArray::fromHex(m_secret);
  m_secret=QAESEncryption::Decrypt(QAESEncryption::AES_256, QAESEncryption::ECB, m_secret, message);
  m_secret=QAESEncryption::RemovePadding(m_secret, QAESEncryption::ISO); //qDebug()<<m_secret;
  }

void TRestApiWorker::setAccessToken(const QByteArray &token)
  {
  m_token=token;
  TSettings().setXmlValue(m_path+"/access_token","",token);
  }

void TRestApiWorker::setSecretKey(const QByteArray &secret)
  {
  m_secret=secret;
  TSettings().setXmlValue(m_path+"/secret_key","",secret);
  }

QByteArray TRestApiWorker::createData(ApiData apiData, const QMap<QString, QVariant> &values, const QByteArray &contentType)
  {
  if (contentType=="application/x-www-form-urlencoded")
    {
    QString data=apiData.pattern;
    if (data.isEmpty()==true)
      {
      QRegularExpression varRegexp("%(.*)%",QRegularExpression::InvertedGreedinessOption);
      QRegularExpressionMatchIterator iterator=varRegexp.globalMatch(data);
      while (iterator.hasNext())
        {
        QRegularExpressionMatch match=iterator.next();
        QString string=match.captured(0); // это типа '%variable%'
        QString key=match.captured(1); // это типа 'variable'
        QString value=values.value(key).toString(); // это значение 'variable', переданное в параметрах

        data.replace(string,value);
        }

      QUrlQuery query;
      foreach (const QString &param, values.keys())
        query.addQueryItem(param,values.value(param).toString());

      data=query.toString();
      }

    return data.toUtf8();
    }

  if (contentType=="application/json")
    {
    QString data=apiData.pattern;
    QRegularExpression varRegexp("%(.*)%",QRegularExpression::InvertedGreedinessOption);
    QRegularExpressionMatchIterator iterator=varRegexp.globalMatch(data);
    while (iterator.hasNext())
      {
      QRegularExpressionMatch match=iterator.next();
      QString string=match.captured(0); // это типа '%variable%'
      QString key=match.captured(1); // это типа 'variable'
      QString value=values.value(key).toString(); // это значение 'variable', переданное в параметрах

      data.replace(string,value);
      }

    return data.toUtf8();
    }

  return QByteArray();
  }

QJsonObject TRestApiWorker::processReply(QNetworkReply *reply)
  {
  reply->deleteLater();

  quint16 code=reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toUInt();
  QNetworkReply::NetworkError errorCode=reply->error();
  if (errorCode!=QNetworkReply::NoError)
    {
    QString errorString=reply->errorString();
    return QJsonObject{{"code",code},{"error_code",errorCode},{"error_string",errorString}};
    }

  QJsonDocument document=QJsonDocument::fromJson(reply->readAll());

  // если пришел JsonObject (нормальная ситуация)
  if (document.isObject()==true)
    {
    QJsonObject object=document.object();
    object.insert("code",code);
    object.insert("error_code",QNetworkReply::NoError);
    return object;
    }

  // если прилетел JsonArray (такое тоже бывает)
  if (document.isArray()==true)
    {
    QJsonArray array=document.array();
    QJsonObject object{{"data",array}};
    object.insert("code",code);
    object.insert("error_code",QNetworkReply::NoError);
    return object;
    }

  return QJsonObject{{"code", 0}, {"error_code",203}, {"error_string", tr("No data")}};
  }

QJsonObject TRestApiWorker::getApiToken()
  {
  return QJsonObject();
  }

QJsonObject TRestApiWorker::sendApiRequest(const QString &method, QMap<QString, QVariant> values, bool sync)
  {
  // если описан метод получения токена и этот (сейчас выполняемый) метод не "получение токена", то сначала выполнить его и получить токен
  if (m_methodMap.contains("GetToken")==true && method!="GetToken")
    {
    QJsonObject response=getApiToken();
    if (response.value("code").toInt()!=200)
      return response;

    m_token=response.value("access_token").toString().toUtf8();
    }

  if (m_baseUrl.isEmpty()==true) // че-то не срослось, этого не должно быть, но...
    {
    QJsonObject response{{"code", -1},{"error_string",QString("Not specified base URL.")}};
    if (sync==false)
      emit answerReceived(response);

    return response;
    }

  ApiData apiData=m_methodMap.value(method);
  QString urlString=apiData.url;
  if (urlString.isEmpty()==true) // че-то не срослось, этого не должно быть, но...
    {
    QJsonObject response{{"code", -1},{"error_string",QString("No URL for API method: %1.").arg(method)}};
    if (sync==false)
      emit answerReceived(response);

    return response;
    }

  // можно добавить для удобства token и secret_key в данные
  values["access_token"]=m_token;
  values["secret_key"]=m_secret;

  QRegularExpression varRegexp("%(.*)%",QRegularExpression::InvertedGreedinessOption);
  QRegularExpressionMatchIterator iterator=varRegexp.globalMatch(urlString);
  while (iterator.hasNext())
    {
    QRegularExpressionMatch match=iterator.next();
    QString string=match.captured(0); // это типа '%variable%'
    QString key=match.captured(1); // это типа 'variable'
    QString value=values.value(key).toString(); // это значение 'variable', переданное в параметрах

    urlString.replace(string,value);
    }

  QUrl url(QUrl(urlString).isRelative()==true ? m_baseUrl+urlString : urlString);
  if (url.isValid()==false)
    {
    QJsonObject response{{"code", -1},{"error",QString("Invalid URL for request: %1.").arg(url.toString())}};
    if (sync==false)
      emit answerReceived(response);

    return response;
    }

  QNetworkRequest request(url);
  request.setAttribute(QNetworkRequest::CacheLoadControlAttribute,QNetworkRequest::AlwaysNetwork);
  request.setAttribute(QNetworkRequest::HttpPipeliningAllowedAttribute,true);

  // заменит в хедерах %(.*)% на реальные значения
  foreach (QString header, apiData.headers)
    {
    iterator=varRegexp.globalMatch(header);
    while (iterator.hasNext())
      {
      QRegularExpressionMatch match=iterator.next();
      QString string=match.captured(0); // это типа '%variable%'
      QString key=match.captured(1); // это типа 'variable'
      QString value=values.value(key).toString(); // это значение 'variable', переданное в параметрах

      header.replace(string,value);
      }

    QStringList parts=header.split(":");
    if (parts.size()<2)
      continue;

    request.setRawHeader(parts.at(0).trimmed().toUtf8(),parts.at(1).trimmed().toUtf8());
    }

  // данные в запрос, заисит от Content-Type
  QByteArray data=createData(apiData,values,request.rawHeader("Content-Type"));
  //qDebug()<<"data to send"<<QString::fromUtf8(data);

  qApp->setOverrideCursor(Qt::WaitCursor);
  QNetworkReply *reply=apiData.httpMethod==TRestApiWorker::Get ? m_manager->get(request) : m_manager->post(request,data);

  // если запрос асинхронный, то коннектим получение ответа и уходим
  if (sync==false)
    {
    connect(reply,&QNetworkReply::finished,this,&TRestApiWorker::apiReplyReceived);
    return QJsonObject();
    }

  QEventLoop loop;
  connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit); // по завершению reply остановить loop
  loop.exec(); // запросы синхронные, надо дождаться завершения

  qApp->restoreOverrideCursor();
  return processReply(reply);
  }

void TRestApiWorker::apiReplyReceived()
  {
  qApp->restoreOverrideCursor();
  QNetworkReply *reply=qobject_cast<QNetworkReply*>(sender());
  if (reply==nullptr)
    return;

  emit answerReceived(processReply(reply));
  }
