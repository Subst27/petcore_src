#pragma once

#include <QObject>
#include <QColor>

class TPalette : public QObject
  {
    Q_OBJECT
  public:
    enum Kind
      {
      Background,       // фон
      Foregroud         // текст
      };
    Q_ENUM(Kind)

    enum Roles
      {
      Saturday=0,       // суббота
      Sunday,           // воскресенье
      Unavailable,      // недоступный
      Missed,           // протухший
      Started,          // начавшийся
      Finished,         // закончившийся
      Waiting,          // время уже подошло, но еще не удаляем, ждем клиента
      Warning           // внимание! напр. Пора напоминать для Напоминаний
      };
    Q_ENUM(Roles)

    explicit TPalette(QObject *parent = nullptr);
    static QColor color(TPalette::Roles role, TPalette::Kind kind);
  };
