#pragma once

#include "TRestApiWorker.h"

class TDadataApiWorker : public TRestApiWorker
  {
    Q_OBJECT

  public:
    explicit TDadataApiWorker(QObject *parent = nullptr);
    static TDadataApiWorker *instance();

  private:
    static TDadataApiWorker *m_instance;
  };

