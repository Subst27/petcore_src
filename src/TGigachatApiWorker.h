#pragma once

#include "TRestApiWorker.h"

class TGigachatApiWorker : public TRestApiWorker
  {
    Q_OBJECT

  public:
    explicit TGigachatApiWorker(QObject *parent = nullptr);
    static TGigachatApiWorker *instance();

    QJsonObject getApiToken() override;

  private:
    static TGigachatApiWorker *m_instance;
    qint64 m_expireMsecs;
  };

