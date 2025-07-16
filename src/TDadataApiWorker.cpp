#include "TDadataApiWorker.h"

TDadataApiWorker *TDadataApiWorker::m_instance=nullptr;

TDadataApiWorker::TDadataApiWorker(QObject *parent)  : TRestApiWorker{parent}
  {
  /*Reserve addresses:
   * "https://suggestions.dadata.ru/suggestions/api/4_1/rs/suggest/address"
   * "https://cleaner.dadata.ru/api/v1/clean/passport"*/
  m_instance=this;

  /* ApiData(HttpMethods method, const QString &url, const QString &pattern, const QStringList &headers) */
  m_methodMap["Address"]=ApiData(Post,"/suggest/address","{\"query\": \"%query%\", \"count\": %count%}",{"Content-Type: application/json",
                                                                                                         "Accept: application/json",
                                                                                                         "Authorization: Token %access_token%"});

  m_methodMap["Passport"]=ApiData(Post,"/clean/passport","[\"%passport%\"]",{"Content-Type: application/json",
                                                                             "Accept: application/json",
                                                                             "Authorization: Token %access_token%",
                                                                             "X-Secret: %secret_key%"});
  }

TDadataApiWorker *TDadataApiWorker::instance()
  {
  return m_instance;
  }
