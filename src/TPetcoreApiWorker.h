#pragma once

#include "TRestApiWorker.h"

class TPetcoreApiWorker : public TRestApiWorker
  {
    Q_OBJECT

  public:
    explicit TPetcoreApiWorker(QObject *parent = nullptr);
    static TPetcoreApiWorker *instance();

  private:
    static TPetcoreApiWorker *m_instance;
  };
