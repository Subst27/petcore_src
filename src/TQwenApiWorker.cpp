#include "TQwenApiWorker.h"

TQwenApiWorker *TQwenApiWorker::m_instance=nullptr;

TQwenApiWorker::TQwenApiWorker(QObject *parent)  : TRestApiWorker{parent}
  {
  m_instance=this;

  /* ApiData(HttpMethods method, const QString &url, const QString &pattern, const QStringList &headers) */
  m_methodMap["Question"]=ApiData(Post,"/chat/completions","{\"model\": \"qwen-plus\", "
                                                           "\"messages\": [{\"role\": \"system\", \"content\": \"%prompt%\"}, "
                                                           "{\"role\": \"user\", \"content\": \"%question%\"}]}",{"Content-Type: application/json",
                                                                                                                  "Authorization: Bearer %access_token%"});
  }

TQwenApiWorker *TQwenApiWorker::instance()
  {
  return m_instance;
  }
