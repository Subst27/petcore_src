#pragma once

#include <QObject>
#include <QMap>
#include <QVariant>
#include <QSqlRecord>

#include <QNetworkRequest>
#include <QNetworkReply>
#include <QJsonObject>

class QNetworkAccessManager;

class TRestApiWorker : public QObject
  {
    Q_OBJECT

  public:
    enum HttpMethods : quint8
      {
      Get,
      Post
      };
    Q_ENUM(HttpMethods)

    // NOTE: может, не pattern, а все же QJsonValue, он примет и объект и строку и массив, или при необходимости вставить Array передать QVariantList ?
    struct ApiData {
      ApiData(HttpMethods httpMethod=Get, const QString &url=QString(), const QString &pattern=QString(),
              const QStringList &headers={"Content-Type: application/json","Authorization: Bearer %access_token%"}) {

        this->httpMethod=httpMethod;
        this->url=url;
        this->pattern=pattern;
        this->headers=headers;
        }

      HttpMethods httpMethod;
      QString url;
      QString pattern;
      QStringList headers;
      };

    explicit TRestApiWorker(QObject *parent = nullptr);
    void initialize(const QString &path);

    void setAccessToken(const QByteArray &token);
    void setSecretKey(const QByteArray &secret);

    virtual QJsonObject getApiToken(); // в каких-то дочках может быть определен, в каких-то нет
    // NOTE: если в ApiData QJsonValue, то и тут не QMap<QString, QVariant>, а QJsonValue, который и пихается в качестве содержимого запроса ?
    QJsonObject sendApiRequest(const QString &method, const QMap<QString, QVariant> &values, bool sync=true);

  protected:
    QString substituteVariables(const QString &source, QMap<QString, QVariant> values);
    //QByteArray createData(TRestApiWorker::ApiData apiData, const QMap<QString, QVariant> &values, const QByteArray &contentType);
    QJsonObject processReply(QNetworkReply *reply);

  protected slots:
    void apiReplyReceived();

  protected:
    QString m_baseUrl;

    QMap<QString,TRestApiWorker::ApiData> m_methodMap;
    QNetworkAccessManager *m_manager;

    QByteArray m_token;
    QByteArray m_secret;

    QString m_path; // путь в конфиге до раздела про API

  signals:
    //void logEvent(TLoggerWorker::Type type,const QString &text);
    void accessTokenExpired();
    void answerReceived(const QJsonObject &response);
  };

Q_DECLARE_METATYPE(TRestApiWorker::HttpMethods)
