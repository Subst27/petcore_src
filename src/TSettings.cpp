#include "TSettings.h"
#include <QDebug>
#include <QSettings>
#include <QFile>
#include <QFileInfo>

#include <QDomNode>

#include <QRect>
#include <QDataStream>

#include <QDateTime>
#include <QColor>
#include <QFont>

#include <QXmlStreamReader>

TSettings::TSettings(const QString &fileName, const QString &info) : m_fileName(fileName), m_info(info), m_changed(false)
  {
  if (qApp->arguments().contains("-develop")==true && QFileInfo(fileName).fileName()=="petcore.xml")
    m_fileName=qApp->applicationDirPath()+"/petcore.xml";

  if (m_info.isEmpty()==true)
    m_info=QFileInfo(m_fileName).baseName();

  QFile file(m_fileName);
  QByteArray data;
  if (file.open(QIODevice::ReadWrite)==true)
    {
    data=file.readAll();
    file.close();
    }

#if QT_VERSION >= QT_VERSION_CHECK(6, 5, 0)
  if ((bool)m_document.setContent(data)==false)
    m_document=QDomDocument();
#else
  if (m_document.setContent(data)==false)
    m_document=QDomDocument();
#endif
  }

TSettings::~TSettings()
  {
  saveSettings();
  }

void TSettings::saveSettings()
  {
  if (m_changed==false)
    return;

  QFile file(m_fileName);
  if (file.open(QIODevice::WriteOnly | QIODevice::Truncate)==false)
    return;

  QTextStream stream(&file);
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
  stream.setEncoding(QStringConverter::Utf8);
#else
  stream.setCodec("UTF-8");
#endif
  stream << m_document.toString(2);
  file.close();

  m_changed=false;
  }

void TSettings::setXmlValue(const QStringList &path, const QString &key, const QVariant &value)
  {
  setInitNode();

  QDomNode node=m_document.firstChildElement();
  QDomElement element;
  foreach (const QString &section,path)
    {
    element=node.firstChildElement(section);
    if (element.isNull()==true)
      {
      element=m_document.createElement(section);
      node=node.appendChild(element);
      }
    else
      node=element;
    }

  QString string=variantToString(value);
  // если key не задан, то value пишется как текст в node, иначе как аттрибут
  // может стоит явно указывать куда писать? типа enum TCommon::valueType (NodeType,AttributeType)
  if (key.isEmpty()==true)
    {
    QDomText text=m_document.createTextNode(string);
    if (node.firstChild().isNull()==true)
      {
      node.appendChild(text);
      m_changed=true;
      return;
      }

    if (text.nodeValue()!=node.firstChild().nodeValue())
      {
      node.replaceChild(text,node.firstChild());
      m_changed=true;
      }

    return;
    }
  // TODO: проверить еще, нет ли такого аттрибута с таким же значением?
  node.toElement().setAttribute(key,string);
  m_changed=true;
  }

void TSettings::setXmlValue(const QString &path, const QString &key, const QVariant &value)
  {
  setXmlValue(path.split("/",Qt::SkipEmptyParts),key,value);
  }

QVariant TSettings::getXmlValue(const QStringList &path, const QString &key, const QVariant &defaultValue)
  {
  QDomNode node=m_document.firstChildElement();
  if (node.isNull()==true || node.toElement().nodeName()!=m_info)
    return defaultValue;

  QDomElement element;
  foreach (const QString &section,path)
    {
    element=node.firstChildElement(section);
    if (element.isNull()==true)
      return defaultValue;
    
    node=element;
    }
  // если key не задан, то value возращается как текст в node, иначе как аттрибут node
  // может стоит явно указывать куда писать? типа enum TCommon::valueType (NodeType,AttributeType)
  if (key.isEmpty()==true)
    {
    if (node.firstChild().isNull()==true) // первым дитем у нода - его значение, текстовый элемент
      return defaultValue;

    return stringToVariant(node.firstChild().toText().data());
    }

  return stringToVariant(node.toElement().attribute(key,defaultValue.toString()));
  }

QVariant TSettings::getXmlValue(const QString &path, const QString &key, const QVariant &defaultValue)
  {
  return getXmlValue(path.split("/",Qt::SkipEmptyParts),key,defaultValue);
  }

void TSettings::writeXmlComment(const QStringList &path, const QString &value)
  {
  setInitNode();

  QDomNode node=m_document.firstChildElement();
  QDomElement element;
  foreach (const QString &section,path)
    {
    element=node.firstChildElement(section);
    if (element.isNull()==true)
      {
      element=m_document.createElement(section);
      node=node.appendChild(element);
      }
    else
      node=element;
    }

  for (quint16 i=0;i<node.childNodes().size();i++)
    {
    QDomNode children=node.childNodes().at(i);
    if (children.isComment()==true && children.toComment().data().simplified()==value.simplified()) // есть такой коммент уже
      return;
    }

  QDomComment comment=m_document.createComment(value);
  node.appendChild(comment);

  m_changed=true;
  }

void TSettings::writeXmlComment(const QString &path, const QString &value)
  {
  writeXmlComment(path.split("/",Qt::SkipEmptyParts),value);
  }

void TSettings::setXmlMaps(const QStringList &path, bool isAttrs, const QList<QMap<QString, QVariant> > &values)
  {
  setInitNode();

  QDomNode node=m_document.firstChildElement();
  QDomElement element;
  QString section;

  for (int i=0;i<path.size()-1;i++)
    {
    section=path.at(i);
    element=node.firstChildElement(section);
    if (element.isNull()==true)
      {
      element=m_document.createElement(section);
      node=node.appendChild(element);
      }
    else
      node=element;
    }
  // грохнуть все child этого узла
  section=path.last();
  while (node.firstChildElement(section).isNull()==false)
    node.removeChild(node.firstChildElement(section));

  // теперь создать заново, исходя из данных values
  for (int i=0;i<values.size();i++)
    {
    QMap <QString,QVariant> attrs=values.at(i);
    element=m_document.createElement(section);
    QDomNode child=node.appendChild(element);

    foreach (const QString &key,attrs.keys())
      {
      QString string=variantToString(attrs.value(key));
      if (isAttrs==true) // если атрибуты
        child.toElement().setAttribute(key,string);
      else // дочерние узлы с текстом
        {
        QDomElement element=m_document.createElement(key);
        QDomText text=m_document.createTextNode(string);
        element.appendChild(text);
        child.appendChild(element);
        }
      }
    }
    
  m_changed=true;
  }

void TSettings::setXmlMaps(const QString &path, bool isAttrs, const QList<QMap<QString, QVariant> > &values)
  {
  setXmlMaps(path.split("/",Qt::SkipEmptyParts),isAttrs,values);
  }

QList<QMap<QString, QVariant> > TSettings::getXmlMaps(const QStringList &path, bool isAttrs)
  {
  QList <QMap <QString,QVariant>> results=QList<QMap <QString,QVariant>>(); // будет возвращаться
  QDomNode node=m_document.firstChildElement();
  if (node.isNull()==true || node.toElement().nodeName()!=m_info)
    return results;

  QDomElement element;
  QString section;
  foreach (section,path)
    {
    element=node.firstChildElement(section);
    if (element.isNull()==true)
      return results;
    
    node=element;
    }

  while (node.isNull()==false)
    {
    QMap <QString,QVariant> values=QMap <QString,QVariant>(); // для текущих значений одного узла
    if (isAttrs==true) // если атрибуты
      {
      QDomNamedNodeMap attributes=node.attributes();
      for (int i=0;i<attributes.size();i++)
        {
        QDomAttr attribute=attributes.item(i).toAttr();
        values.insert(attribute.name(),stringToVariant(attribute.value()));
        }
      }
    else
      {
      QDomNodeList nodes=node.childNodes();// тут читаем дочерние узлы и их тексты
      for (int i=0;i<nodes.size();i++)
        {
        QDomNode child=nodes.item(i);
        values.insert(child.nodeName(),stringToVariant(child.firstChild().toText().data()));
        }
      }
			
    results<<values;
    node=node.nextSiblingElement(section);
    }
		
  return results;
  }

QList<QMap<QString, QVariant> > TSettings::getXmlMaps(const QString &path, bool isAttrs)
  {
  return getXmlMaps(path.split("/",Qt::SkipEmptyParts),isAttrs);
  }
//
void TSettings::removeXmlEntry(const QStringList &path)
  {
  QDomNode node=m_document.firstChildElement();
  if (node.isNull()==true || node.toElement().nodeName()!=m_info)
    return;

  QDomElement element;
  foreach (const QString &section,path)
    {
    element=node.firstChildElement(section);
    if (element.isNull()==true)
      return;
    
    node=element;
    }

  node.parentNode().removeChild(node);
  m_changed=true;
  }

void TSettings::removeXmlEntry(const QString &path)
  {
  removeXmlEntry(path.split("/",Qt::SkipEmptyParts));
  }

void TSettings::removeXmlComment(const QStringList &path, const QString &value)
  {
  QDomNode node=m_document.firstChildElement();
  if (node.isNull()==true || node.toElement().nodeName()!=m_info)
    return;

  QDomElement element;
  foreach (const QString &section,path)
    {
    element=node.firstChildElement(section);
    if (element.isNull()==true)
      return;
    
    node=element;
    }

  for (qint16 i=node.childNodes().size()-1;i>-1;i--)
    {
    QDomNode children=node.childNodes().at(i);
    if (children.isComment()==true && children.toComment().data().simplified()==value.simplified()) // есть такой коммент
      {
      node.removeChild(children);
      m_changed=true;
      }
    }
  }

void TSettings::removeXmlComment(const QString &path, const QString &value)
  {
  removeXmlComment(path.split("/",Qt::SkipEmptyParts),value);
  }

bool TSettings::containsXmlValue(const QStringList &path, const QString &key)
  {
  QDomNode node=m_document.firstChildElement();
  if (node.isNull()==true || node.toElement().nodeName()!=m_info)
    return false;

  QDomElement element;
  foreach (const QString &section,path)
    {
    element=node.firstChildElement(section);
    if (element.isNull()==true)
      return false;
    
    node=element;
    }
  // если key не задан, то value возращается как текст в node, иначе как аттрибут node
  // может стоит явно указывать куда писать? типа enum TCommon::valueType (NodeType,AttributeType)
  if (key.isEmpty()==true)
    return (node.firstChild().isNull()==false);
 
  return (node.toElement().hasAttribute(key)==true);
  }

bool TSettings::containsXmlValue(const QString &path, const QString &key)
  {
  return containsXmlValue(path.split("/",Qt::SkipEmptyParts),key);
  }



QList<TSettings::Node> TSettings::getAllSettings(QString fileName, QString info)
  {
  QList <Node> result;

  QDomDocument document("");
  if (QFileInfo(fileName).path().isEmpty()==true)
    fileName.prepend(qApp->applicationDirPath()+"/");
  
  if (info.isEmpty()==true)
    info=QFileInfo(fileName).baseName();

  QFile file(fileName);
  QByteArray data;
  if (file.open(QIODevice::ReadOnly)==true)
    {
    data=file.readAll();
    file.close();
    }

#if QT_VERSION >= QT_VERSION_CHECK(6, 5, 0)
  if ((bool)document.setContent(data)==false) // неверный XML
    return result;
#else
  if (document.setContent(data)==false) // неверный XML
    return result;
#endif

  QDomNode node=document.firstChildElement();
  if (node.isNull()==true || node.toElement().nodeName()!=info) // не наш конфиг
    return result;

  QXmlStreamReader xmlReader(data);
  QStringList path;

  // надо добраться до первого Элемента, он как бы корень для нас тут
  while (xmlReader.atEnd()==false && xmlReader.isStartElement()==false)
    xmlReader.readNext();

  bool comment=false;
  while (xmlReader.atEnd()==false)
    {
    xmlReader.readNext();
    QString key;
    QString value;

    if (xmlReader.isStartElement()==true)
      {
      key=xmlReader.name().toString();
      path.append(key);
      xmlReader.readNext();
      value=xmlReader.text().toString().simplified();
      comment=false;
      }

    if (xmlReader.isComment()==true)
      {
      key=xmlReader.text().toString();
      value=xmlReader.text().toString();
      comment=true;
      }

    if (key.isEmpty()==false && value.isEmpty()==false)
      result<<Node(path.join("/"),stringToVariant(value),comment);

    if (xmlReader.isEndElement()==true && path.size()>0)
      path.removeLast();
    }

  return result;
  }

void TSettings::setInitNode()
  {
  QDomNode node=m_document.firstChildElement();
  if (node.isNull()==true || node.toElement().nodeName()!=m_info)
    {
    m_document.clear();
    m_document.appendChild(m_document.createProcessingInstruction("xml","version='1.0' encoding='UTF-8'"));
    m_document.appendChild(m_document.createComment(QString("The %1 settings").arg(m_info)));
    node=m_document.createElement(m_info);
    m_document.appendChild(node);
    }
  }

QString TSettings::variantToString(const QVariant &variant)
  {
  QString string;
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
  switch (variant.typeId())
#else
  switch ((QMetaType::Type)variant.type())
#endif
    {
    case QMetaType::UnknownType:
      {
      string=QLatin1String("@Invalid()");
      break;
      }
    case QMetaType::QByteArray:
      {
      QByteArray bytes=variant.toByteArray().toHex().toUpper().prepend("0x");
      string=QLatin1String("@ByteArray(")+QLatin1String(bytes.constData(),bytes.size())+QLatin1Char(')');
      break;
      }
    case QMetaType::QString:
    case QMetaType::QChar:
    case QMetaType::Long:
    case QMetaType::LongLong:
    case QMetaType::ULongLong:
    case QMetaType::Int:
    case QMetaType::UInt:
    case QMetaType::Short:
    case QMetaType::UShort:
    case QMetaType::Char:
    case QMetaType::UChar:
    case QMetaType::SChar:
    case QMetaType::Bool:
    case QMetaType::Float:
    case QMetaType::Double:
    case QMetaType::QKeySequence:
      {
      string=variant.toString();
      if (string.contains(QChar::Null))
        string=QLatin1String("@String(")+string+QLatin1Char(')');

      if (string.startsWith(QLatin1Char('@')))
        string.prepend(QLatin1Char('@'));
      break;
      }
    /*case QMetaType::QLine:
    case QMetaType::QLineF:*/
    case QMetaType::QRect:
      {
      QRect rect=qvariant_cast<QRect>(variant);
      string=QString("@Rect(%1,%2,%3,%4)").arg(rect.x()).arg(rect.y()).arg(rect.width()).arg(rect.height());
      break;
      }
    case QMetaType::QRectF:
      {
      QRectF rect=qvariant_cast<QRectF>(variant);
      string=QString("@RectF(%1,%2,%3,%4)").arg(rect.x(),0,'f',3).arg(rect.y(),0,'f',3).arg(rect.width(),0,'f',3).arg(rect.height(),0,'f',3);
      break;
      }
    case QMetaType::QSize:
      {
      QSize size=qvariant_cast<QSize>(variant);
      string=QString("@Size(%1,%2)").arg(size.width()).arg(size.height());
      break;
      }
    case QMetaType::QSizeF:
      {
      QSizeF size=qvariant_cast<QSizeF>(variant);
      string=QString("@SizeF(%1,%2)").arg(size.width(),0,'f',3).arg(size.height(),0,'f',3);
      break;
      }
    case QMetaType::QPoint:
      {
      QPoint point=qvariant_cast<QPoint>(variant);
      string=QString("@Point(%1,%2)").arg(point.x()).arg(point.y());
      break;
      }
    case QMetaType::QPointF:
      {
      QPointF point=qvariant_cast<QPointF>(variant);
      string=QString("@PointF(%1,%2)").arg(point.x(),0,'f',3).arg(point.y(),0,'f',3);
      break;
      }
    case QMetaType::QDate:
      {
      QDate date=qvariant_cast<QDate>(variant);
      string=QString("@Date(%1,%2,%3)").arg(date.year()).arg(date.month()).arg(date.day());
      break;
      }
    case QMetaType::QTime:
      {
      QTime time=qvariant_cast<QTime>(variant);
      string=QString("@Time(%1,%2,%3,%4)").arg(time.hour()).arg(time.minute()).arg(time.second()).arg(time.msec());
      break;
      }
    case QMetaType::QDateTime:
      {
      QDateTime dateTime=qvariant_cast<QDateTime>(variant);
      string=QString("@DateTime(%1,%2,%3,%4,%5,%6,%7,%8)").arg(dateTime.date().year()).arg(dateTime.date().month()).arg(dateTime.date().day()).
             arg(dateTime.time().hour()).arg(dateTime.time().minute()).arg(dateTime.time().second()).arg(dateTime.time().msec()).arg(dateTime.timeSpec());
      break;
      }
    case QMetaType::QColor:
      {
      QColor color=qvariant_cast<QColor>(variant);
      string=QString("@Color(0x%1)").arg(QString::number(color.rgba(),16).toUpper());
      break;
      }
    case QMetaType::QFont:
      {
      QFont font=qvariant_cast<QFont>(variant);
      string="@Font("+font.toString()+")";
      break;
      }
    case QMetaType::QStringList:
      {
      string="@StringList("+variant.toStringList().join(";")+")";
      break;
      }
      /*case QMetaType::List:
          {
          string="@List("+variant.toList().join(";")+")";
          break;
          }*/
    default:
      {
#ifndef QT_NO_DATASTREAM
      QByteArray bytes;
      QDataStream stream(&bytes,QIODevice::WriteOnly);
      stream<<variant;
      bytes=bytes.toHex().toUpper().prepend("0x");
      string=QLatin1String("@Variant(")+QLatin1String(bytes.constData(),bytes.size())+QLatin1Char(')');
#else
      Q_ASSERT(!"QSettings: Cannot save custom types without QDataStream support");
#endif
      break;
      }
    }

  return string;
  }

QVariant TSettings::stringToVariant(const QString &string)
  {
  QStringList args;
  if (string.startsWith(QLatin1Char('@'))==true && string.endsWith(QLatin1Char(')'))==true)
    {
    if (string.startsWith(QLatin1String("@ByteArray(0x"))==true)
      return QVariant(QByteArray::fromHex(string.mid(13,string.size()-14).toLatin1()));

    if (string.startsWith(QLatin1String("@String("))==true)
      return QVariant(string.mid(8,string.size()-9));

    if (string.startsWith(QLatin1String("@Rect("))==true)
      {
      args=string.mid(6,string.size()-7).split(",");
      if (args.size()==4)
        return QVariant(QRect(args[0].toInt(),args[1].toInt(),args[2].toInt(),args[3].toInt()));
      }

    if (string.startsWith(QLatin1String("@RectF("))==true)
      {
      args=string.mid(7,string.size()-8).split(",");
      if (args.size()==4)
        return QVariant(QRectF(args[0].toFloat(),args[1].toFloat(),args[2].toFloat(),args[3].toFloat()));
      }

    if (string.startsWith(QLatin1String("@Size("))==true)
      {
      args=string.mid(6,string.size()-7).split(",");
      if (args.size()==2)
        return QVariant(QSize(args[0].toInt(),args[1].toInt()));
      }

    if (string.startsWith(QLatin1String("@SizeF("))==true)
      {
      args=string.mid(7,string.size()-8).split(",");
      if (args.size()==2)
        return QVariant(QSizeF(args[0].toFloat(),args[1].toFloat()));
      }

    if (string.startsWith(QLatin1String("@Point("))==true)
      {
      args=string.mid(7,string.size()-8).split(",");
      if (args.size()==2)
        return QVariant(QPoint(args[0].toInt(),args[1].toInt()));
      }

    if (string.startsWith(QLatin1String("@PointF("))==true)
      {
      args=string.mid(8,string.size()-9).split(",");
      if (args.size()==2)
        return QVariant(QPointF(args[0].toFloat(),args[1].toFloat()));
      }

    if (string.startsWith(QLatin1String("@Date("))==true)
      {
      args=string.mid(6,string.size()-7).split(",");
      if (args.size()==3)
        return QVariant(QDate(args[0].toInt(),args[1].toInt(),args[2].toInt()));
      }

    if (string.startsWith(QLatin1String("@Time("))==true)
      {
      args=string.mid(6,string.size()-7).split(",");
      if (args.size()==4)
        return QVariant(QTime(args[0].toInt(),args[1].toInt(),args[2].toInt(),args[3].toInt()));
      }

    if (string.startsWith(QLatin1String("@DateTime("))==true)
      {
      args=string.mid(10,string.size()-11).split(",");
      if (args.size()==8)
        return QVariant(QDateTime(QDate(args[0].toInt(),args[1].toInt(),args[2].toInt()),
                        QTime(args[3].toInt(),args[4].toInt(),args[5].toInt(),args[6].toInt()),Qt::TimeSpec(args[7].toInt())));
      }

    if (string.startsWith(QLatin1String("@Color("))==true)
      return QVariant(QColor(string.mid(7,string.size()-8).remove("0x").toUInt(nullptr,16)));

    if (string.startsWith(QLatin1String("@Font(")))
      {
      QFont font;
      if (font.fromString(string.mid(6,string.size()-7)))
        return QVariant(font);
      }

    if (string.startsWith(QLatin1String("@StringList("))==true)
      return QVariant(string.mid(12,string.size()-13).split(";",Qt::SkipEmptyParts));

    if (string.startsWith(QLatin1String("@Variant(0x"))==true)
      {
#ifndef QT_NO_DATASTREAM
      QByteArray bytes=QByteArray::fromHex(string.mid(11,string.size()-12).toLatin1());
      QDataStream stream(&bytes,QIODevice::ReadOnly);
      QVariant variant;
      stream >> variant;
      return variant;
#else
      Q_ASSERT(!"QSettings: Cannot load custom types without QDataStream support");
#endif
      }
    if (string==QLatin1String("@Invalid()"))
      return QVariant();

    if (string.startsWith(QLatin1String("@@")))
      return QVariant(string.mid(1));
    }

  return QVariant(string);
  }
