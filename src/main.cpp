#include <QDebug>

#include <QApplication>
#include <QTranslator>
#include <QStyleFactory>

#include <QProcessEnvironment>
#include <QStyleHints>

#include <QDBusInterface>
#include <QDBusConnection>

#include "TMainWnd.h"

int main(int argc, char *argv[])
  {
  QApplication App(argc, argv);

  App.setStyle(QStyleFactory::create("Fusion"));

  QColor backgroundColor=qApp->palette().base().color().toHsl();
  QColor foregroundColor=qApp->palette().text().color().toHsl();

  QString theme=backgroundColor.lightness() < foregroundColor.lightness() ? "dark" : "light";
  App.setProperty("system_theme",theme);

  QString translatePath;
#ifdef Q_OS_WINDOWS
  translatePath=qApp->applicationDirPath()+"/translations/";
#elif defined(Q_OS_LINUX)
  translatePath=qApp->applicationDirPath()+"/../share/translations/";
#elif defined(Q_OS_MACOS)
  translatePath=qApp->applicationDirPath()+"/../Resources/translations/";
#endif

  QTranslator mainTranslator;
  if (mainTranslator.load(translatePath+"petcore_"+QLocale::system().name()))
    App.installTranslator(&mainTranslator);

  QTranslator lrTranslator;
  if (lrTranslator.load(translatePath+"limereport_"+QLocale::system().name()))
    App.installTranslator(&lrTranslator);

  QTranslator baseTranslator;
  if (baseTranslator.load(translatePath+"qtbase_"+QLocale::system().name()))
    App.installTranslator(&baseTranslator);

  QTranslator designerTranslator;
  if (designerTranslator.load(translatePath+"designer_"+QLocale::system().name()))
    App.installTranslator(&designerTranslator);

  TMainWnd MainWnd;
  MainWnd.show();
  return App.exec();
  }
