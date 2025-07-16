#include "TPetcoreApiWorker.h"

TPetcoreApiWorker *TPetcoreApiWorker::m_instance=nullptr;

TPetcoreApiWorker::TPetcoreApiWorker(QObject *parent)  : TRestApiWorker{parent}
  {
  m_instance=this;

  /* ApiData(HttpMethods method, const QString &url, const QString &pattern, const QStringList &headers) */
  // авторизация и т.п.
  m_methodMap["Authorize"]=ApiData(Post,"/login/auth","{\"email\": \"%login%\", \"password\": \"%password%\"}",{"Content-Type: application/json"});
  m_methodMap["Logout"]=ApiData(Get,"/login/logout","",{"Content-Type: application/json","Authorization: Bearer %access_token%"});
  m_methodMap["CheckToken"]=ApiData(Get,"/checktoken","",{"Content-Type: application/json","Authorization: Bearer %access_token%"});

  /* отправка в телегу через сервер */
  m_methodMap["SendTelegram"]=ApiData(Post,"/v1/send/tgmessage","{\"tg_id\": \"%telegram%\", \"phone\": \"%phone_number%\","
                                                                 "\"name\": \"%name%\", \"text\": \"%message%\"}",{"Content-Type: application/json",
                                                                                                               "Authorization: Bearer %access_token%"});
  /* Справочники */
  // виды животных и породы
  m_methodMap["GetSpecies"]=ApiData(Get,"/v1/data/animals","",{"Content-Type: application/json","Authorization: Bearer %access_token%"});
  m_methodMap["GetBreeds"]=ApiData(Get,"/v1/data/breeds/%species_id%","",{"Content-Type: application/json","Authorization: Bearer %access_token%"});
  // статусы, цели посещений и типы маркировки
  m_methodMap["GetStatuses"]=ApiData(Get,"/v1/data/petstatuses","",{"Content-Type: application/json","Authorization: Bearer %access_token%"});
  m_methodMap["GetMarking"]=ApiData(Get,"/v1/data/makrtypes","",{"Content-Type: application/json","Authorization: Bearer %access_token%"});
  // специализации врачей
  m_methodMap["GetProfiles"]=ApiData(Get,"/v1/data/specializations","",{"Content-Type: application/json","Authorization: Bearer %access_token%"});
  // учены звания и ученые степени врачей
  m_methodMap["GetDegrees"]=ApiData(Get,"/v1/data/academicdegrees","",{"Content-Type: application/json","Authorization: Bearer %access_token%"});
  m_methodMap["GetRanks"]=ApiData(Get,"/v1/data/ranks","",{"Content-Type: application/json","Authorization: Bearer %access_token%"});
  // специализации по возрастам животных и по направлению деятельности
  m_methodMap["GetPetages"]=ApiData(Get,"/v1/data/petages","",{"Content-Type: application/json","Authorization: Bearer %access_token%"});
  m_methodMap["GetActions"]=ApiData(Get,"/v1/data/procedures","",{"Content-Type: application/json","Authorization: Bearer %access_token%"});
  }

TPetcoreApiWorker *TPetcoreApiWorker::instance()
  {
  return m_instance;
  }
