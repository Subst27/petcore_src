#include <QApplication>
#include <QTranslator>
#include <QStyleFactory>

#include "TMainWnd.h"

int main(int argc, char *argv[])
  {
  QApplication App(argc, argv);

  App.setStyle(QStyleFactory::create("Fusion"));

  QTranslator mainTranslator;
  if (mainTranslator.load(qApp->applicationDirPath()+"/translations/petcore_"+QLocale::system().name()))
    App.installTranslator(&mainTranslator);

  QTranslator qtTranslator;
  if (qtTranslator.load(qApp->applicationDirPath()+"/translations/qtbase_"+QLocale::system().name()))
    App.installTranslator(&qtTranslator);

  TMainWnd MainWnd;
  MainWnd.show();
  return App.exec();
  }
