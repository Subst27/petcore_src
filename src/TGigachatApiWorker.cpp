#include "TGigachatApiWorker.h"
#include "TSettings.h"

#include <QSysInfo>

TGigachatApiWorker *TGigachatApiWorker::m_instance=nullptr;

TGigachatApiWorker::TGigachatApiWorker(QObject *parent)  : TRestApiWorker{parent}, m_expireMsecs(0)
  {
  m_instance=this;

  /* ApiData(HttpMethods method, const QString &url, const QString &pattern, const QStringList &headers) */
  m_methodMap["GetToken"]=ApiData(Post,"https://ngw.devices.sberbank.ru:9443/api/v2/oauth",
                                       "scope=GIGACHAT_API_PERS",{"Content-Type: application/x-www-form-urlencoded",
                                                                  "Accept: application/json",
                                                                  "RqUID: %uuid%",
                                                                  "Authorization: Basic %secret_key%"});

  m_methodMap["Question"]=ApiData(Post,"/chat/completions","{\"model\": \"GigaChat-2-Pro:latest\", "
                                                           "\"messages\": [{\"role\": \"system\", \"content\": \"%prompt%\"}, "
                                                           "{\"role\": \"user\", \"content\": \"%question%\"}]}",{"Content-Type: application/json",
                                                                                                                  "Authorization: Bearer %access_token%"});
  }

TGigachatApiWorker *TGigachatApiWorker::instance()
  {
  return m_instance;
  }

QJsonObject TGigachatApiWorker::getApiToken()
  {
  // если токен не истек и не истечет еще хотя бы 10 сек (на всякий случай)
  if (m_expireMsecs-QDateTime::currentMSecsSinceEpoch()>9999)
    return {{"code", 200},{"error_code", 0},{"access_token", QString::fromUtf8(m_token)},{"expire_at", m_expireMsecs}};

  QJsonObject response=sendApiRequest("GetToken",{{"secret_key", m_secret}, {"uuid", QSysInfo::machineUniqueId()}},true);
  m_expireMsecs=response.value("expires_at").toVariant().toLongLong();

  return response;
  }
