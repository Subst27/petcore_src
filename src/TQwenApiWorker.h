#pragma once

#include "TRestApiWorker.h"

class TQwenApiWorker : public TRestApiWorker
  {
    Q_OBJECT

  public:
    explicit TQwenApiWorker(QObject *parent = nullptr);
    static TQwenApiWorker *instance();

  private:
    static TQwenApiWorker *m_instance;
  };

