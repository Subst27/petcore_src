#pragma once

#include <QByteArray>
#include <QString>
#include <QVariant>
#include <QApplication>

#include <QDomDocument>
#include <QFile>

#include <QStandardPaths>

class TSettings
  {
    //Q_OBJECT
  public:
    struct Node final
      {
      Node(const QString &path=QString(), const QVariant &value=QVariant(), bool comment=false){
        this->path=path;
        this->value=value;
        this->comment=comment;
        }

      QString path;
      QVariant value;
      bool comment;
      QString toString() const {
        return "("+path+", "+variantToString(value)+", "+(comment ? "true)" : "false)");
        }
      };

    explicit TSettings(const QString &fileName=QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation)+"/petcore.xml", const QString &info="petcore");
    ~TSettings();

    void saveSettings();

    void setXmlValue(const QStringList &path, const QString &key, const QVariant &value);
    void setXmlValue(const QString &path, const QString &key, const QVariant &value); // оставить только такие методы, с const QString &path ?

    QVariant getXmlValue(const QStringList &path, const QString &key, const QVariant &defaultValue=QVariant());
    QVariant getXmlValue(const QString &path,const QString &key,const QVariant &defaultValue=QVariant());

    void writeXmlComment(const QStringList &path, const QString &value);
    void writeXmlComment(const QString &path,const QString &value);

    void setXmlMaps(const QStringList &path, bool isAttrs, const QList<QMap<QString,QVariant>> &values);
    void setXmlMaps(const QString &path, bool isAttrs, const QList<QMap<QString,QVariant>> &values);

    QList<QMap<QString, QVariant>> getXmlMaps(const QStringList &path,bool isAttrs);
    QList<QMap<QString, QVariant>> getXmlMaps(const QString &path, bool isAttrs);

    void removeXmlEntry(const QStringList &path);
    void removeXmlEntry(const QString &path);

    void removeXmlComment(const QStringList &path, const QString &value);
    void removeXmlComment(const QString &path, const QString &value);

    bool containsXmlValue(const QStringList &path, const QString &key);
    bool containsXmlValue(const QString &path,const QString &key);
    // xml файл, список всех конечных значений вместе с путями + комменты также возвращает
    static QList <Node> getAllSettings(QString fileName, QString info);

  protected:
    void setInitNode();
    // то для удобства представления, видеть смысл записей просто открыв XML
    static QString variantToString(const QVariant &variant);
    static QVariant stringToVariant(const QString &string);
		
  private:
   QString m_fileName;
   QString m_info;

   QDomDocument m_document;
   bool m_changed;
  };
