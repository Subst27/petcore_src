#include "TPalette.h"

#include <QApplication>
#include <QPalette>

TPalette::TPalette(QObject *parent) : QObject{parent}
  {
  /*   Constant   |                Background                |    Foreground
   * -------------+------------------------------------------+-------------------
   * Saturday     |  QColor("#9DE2FF") или QColor("#E0DBFF") | QPalette Dark
   * Sunday       |  QColor("#29BFFF") или QColor("#ABBDFF") | QPalette Dark
   * Unavailable  |  QColor("#D8E5EC") или QColor("#D9E5EC") | QColor("#7C7C7C")
   * Missed       |  QColor("#B5E9FF") или QColor("#F1A3FF") | QColor("#7C7C7C")
   * Started      |  QColor("#73B4D0") или QColor("#6D82D3") | QPalette Light
   * Finished     |  QColor("#B2C6D0") или QColor("#C8B2D0") | QPalette Light
   * Waiting      |  QColor("#AE008C") или QColor("#AE008C") | QColor("#7C7C7C")
   * Warning      |  QColor("#0092D0") или QColor("#E635C4") | QPalette Light
   * -------------+------------------------------------------+------------------- */
  }

QColor TPalette::color(TPalette::Roles role, TPalette::Kind kind)
  {
  QString theme=qApp->property("app_theme").toString();
  QColor darkColor=(theme=="light" ? qApp->palette().windowText().color() : qApp->palette().light().color());
  QColor lightColor=(theme=="light" ? qApp->palette().light().color() : qApp->palette().windowText().color());
  switch (role)
    {
    case Saturday:
      return (kind==TPalette::Background ? QColor("#9DE2FF") : darkColor);
    case Sunday:
      return (kind==TPalette::Background ? QColor("#29BFFF") : darkColor);
    case Unavailable:
      return (kind==TPalette::Background ? QColor("#D8E5EC") : QColor("#7C7C7C"));
    case Missed:
      return (kind==TPalette::Background ? QColor("#B5E9FF") : QColor("#7C7C7C"));
    case Started:
      return (kind==TPalette::Background ? QColor("#73B4D0") : lightColor);
    case Finished:
      return (kind==TPalette::Background ? QColor("#B2C6D0") : lightColor);
    case Waiting:
      return (kind==TPalette::Background ? QColor("#AE008C") : QColor("#7C7C7C"));
    case Warning:
      return (kind==TPalette::Background ? QColor("#0092D0") : lightColor);
    default:
      return (kind==TPalette::Background ? qApp->palette().window().color() : qApp->palette().windowText().color());
    }

  return (kind==TPalette::Background ? qApp->palette().window().color() : qApp->palette().windowText().color());
  }
