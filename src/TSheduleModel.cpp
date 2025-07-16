#include "TSheduleModel.h"

#include <QDebug>

#include <QDate>

TSheduleModel::TSheduleModel(QObject *parent) : QAbstractItemModel(parent)
  {
  m_headers=QStringList{tr("Title") ,tr("Date"), tr("Doctor ID"), tr("Time"), tr("Data"), tr("Type")};
  m_fields=QStringList{"title", "date", "doctor_id", "time", "data", "type"};

  if (m_fields.size()!=m_headers.size() || m_fields.size()!=(quint8)TSheduleModel::Type+1)
    qWarning("TSheduleModel: sizes of headers and fields seems invalid");

  m_root=ItemInfo(nullptr,QVector <ItemInfo*>(),{});
  m_rootIndex=index(0,TSheduleModel::Title,QModelIndex());

  emit dataChanged(m_rootIndex,m_rootIndex);
  }

TSheduleModel::~TSheduleModel()
  {
  clear(m_rootIndex);
  }

void TSheduleModel::setPeriod(const QDate &fromDate, const QDate &toDate)
  {
  if (m_fromDate==fromDate && toDate==m_toDate)
    return;

  m_fromDate=fromDate;
  m_toDate=toDate;

  clear(m_rootIndex);
  for (QDate date=fromDate; date<=toDate; date=date.addDays(1))
    {
    insertRow(-1,m_rootIndex);
    quint16 row=rowCount(m_rootIndex)-1;

    setData(index(row,TSheduleModel::Title,m_rootIndex),"Title");
    setData(index(row,TSheduleModel::Date,m_rootIndex),date);
    setData(index(row,TSheduleModel::DoctorId,m_rootIndex),0);
    setData(index(row,TSheduleModel::Time,m_rootIndex),QTime(23,59,59));
    setData(index(row,TSheduleModel::Type,m_rootIndex),TSheduleModel::DateType);
    setData(index(row,TSheduleModel::DataRecord,m_rootIndex),QHash<QString,QVariant>());
    }

  setHeaderData(TSheduleModel::Title, Qt::Horizontal, tr("Date"), Qt::DisplayRole);
  }

QPair<QDate, QDate> TSheduleModel::period() const
  {
  return QPair<QDate, QDate>(m_fromDate,m_toDate);
  }

QVariantList TSheduleModel::record(const QModelIndex &parent, quint16 row, int role) const
  {
  QVariantList record;
  for (quint8 i=0;i<columnCount();i++)
    record << data(index(row,i,parent),role);

  return record;
  }

/*QStringList TSheduleModel::fields() const
  {
  return m_fields;
  }

qint8 TSheduleModel::fieldIndex(const QString &fieldName) const
  {
  if (m_fields.contains(fieldName)==false)
    qWarning("TSheduleModel: no such field '%s'", fieldName.toLocal8Bit().data());

  return m_fields.indexOf(fieldName);
  }*/

QModelIndex TSheduleModel::index(int row, int column, const QModelIndex &parent) const
  {
  if (hasIndex(row,column,parent)==false)
    return QModelIndex();

  if (parent.isValid()==false)
    return createIndex(row,column,const_cast <ItemInfo*>(&m_root));

  ItemInfo *parentInfo=static_cast <ItemInfo*>(parent.internalPointer());

  if (parentInfo->children.size()>row)
    return createIndex(row,column,parentInfo->children.at(row));

  return QModelIndex();
  }

QModelIndex TSheduleModel::parent(const QModelIndex &index) const
  {
  if (index.isValid()==false)
    return QModelIndex();

  ItemInfo *itemInfo=static_cast <ItemInfo*>(index.internalPointer());
  ItemInfo *parentInfo=itemInfo->parent;

  if (parentInfo==nullptr)
    return QModelIndex();

  if (parentInfo->parent==nullptr)
    return createIndex(0,0,parentInfo);

  QVector <ItemInfo*> parentInfoChildren=parentInfo->parent->children;
  return createIndex(parentInfoChildren.indexOf(parentInfo),0,parentInfo);
  }

int TSheduleModel::rowCount(const QModelIndex &parent) const
  {
  if (parent.isValid()==false)
    return 1;

  ItemInfo *parentInfo=static_cast <ItemInfo*>(parent.internalPointer());
  return parentInfo->children.size();
  }

int TSheduleModel::columnCount(const QModelIndex &parent) const
  {
  Q_UNUSED(parent);
  return m_fields.size();
  }

QVariant TSheduleModel::data(const QModelIndex &index,int role) const
  {
  ItemInfo *itemInfo=static_cast <ItemInfo*>(index.internalPointer());
  if (index.isValid()==false || itemInfo->data.size()==0)
    return QVariant();

  switch (role)
    {
    case Qt::DisplayRole:
    case Qt::EditRole:
      return itemInfo->data.at(index.column());

    default:
      return QVariant();
    }
  }

bool TSheduleModel::setData(const QModelIndex &index,const QVariant &value,int role)
  {
  ItemInfo *itemInfo=static_cast <ItemInfo*> (index.internalPointer());
  if (index.isValid()==false || itemInfo->data.size()==0)
    return false;

  if (role==Qt::EditRole)
    itemInfo->data[index.column()]=value;

  emit dataChanged(index,index.sibling(index.row(),columnCount()));
  return true;
  }

Qt::ItemFlags TSheduleModel::flags(const QModelIndex &index) const
  {
  Qt::ItemFlags flags=QAbstractItemModel::flags(index);
  flags.setFlag(Qt::ItemIsEditable,false);

  return flags;
  }

QVariant TSheduleModel::headerData(int section,Qt::Orientation orientation,int role) const
  {
  if (orientation!=Qt::Horizontal || section>=m_fields.size())
    return QVariant();

  switch (role)
    {
    case Qt::DisplayRole:
      return m_headers.at(section);
    case Qt::EditRole:
    case Qt::UserRole:
      return m_fields.at(section);
    default:
      return QVariant();
    }
  }

bool TSheduleModel::setHeaderData(int section,Qt::Orientation orientation,const QVariant &value,int role)
  {
  if (orientation!=Qt::Horizontal || section>=m_fields.size())
    return false;

  switch (role)
    {
    case Qt::DisplayRole:
      {
      m_headers[section]=value.toString();
      break;
      }
    case Qt::EditRole:
    case Qt::UserRole:
      {
      m_fields[section]=value.toString();
      break;
      }
    default:
      return false;
    }

  emit headerDataChanged(orientation,section,section);
  return true;
  }

int TSheduleModel::columnPosition(const QString &name)
  {
  return m_fields.indexOf(name);
  }

bool TSheduleModel::insertColumns(int position,int columns,const QModelIndex &parent)
  {
  beginInsertColumns(parent,position,position+columns-1);
  // TODO: во все itemInfo.data пихнуть новые данные, пустые
  endInsertColumns();
  return QAbstractItemModel::insertColumns(position,columns,parent);
  }

bool TSheduleModel::removeColumns(int position,int columns,const QModelIndex &parent)
  {
  beginRemoveColumns(parent,position,position+columns-1);
  // TODO: из всех itemInfo.data убить данные
  endRemoveColumns();
  return QAbstractItemModel::removeColumns(position,columns,parent);
  }

bool TSheduleModel::insertRows(int position,int rows,const QModelIndex &parent)
  {
  ItemInfo *parentInfo=parent.isValid() ? static_cast<ItemInfo*>(parent.internalPointer()) : &m_root;
  if (position==-1 || position>parentInfo->children.size())
    position=parentInfo->children.size();

  beginInsertRows(parent,position,position+rows-1);
  parentInfo->children.reserve(parentInfo->children.size()+rows);
  for (qint16 i=position;i<position+rows;i++)
    {
    ItemInfo *itemInfo=new ItemInfo(parentInfo,QVector<ItemInfo *>(),QVector<QVariant>(columnCount(),QVariant()));
    parentInfo->children.insert(position,itemInfo);
    }
  endInsertRows();

  emit dataChanged(index(position,0,parent),index(position+rows-1,columnCount(),parent));
  return true;
  }

bool TSheduleModel::removeRows(int position,int rows,const QModelIndex &parent)
  {
  ItemInfo *parentInfo=parent.isValid() ? static_cast<ItemInfo*>(parent.internalPointer()) : &m_root;
  if (position==-1 || rows==0 || position+rows>parentInfo->children.size())
    return false;

  //Мне нужно пройти по всему дереву и грохнуть все дочерние и дочернии дочерних и т.д., можно рекусрией, можно напрямую
  QVector <ItemInfo*> childInfo=parentInfo->children;
  for (qint16 i=position+rows-1;i>=position;i--)
    {
    quint16 childSize=childInfo.at(i)->children.size();
    if (childSize>0)
      removeRows(0,childSize,index(i,0,parent));

    beginRemoveRows(parent,i,i);
    parentInfo->children.remove(i);
    delete childInfo.at(i);
    endRemoveRows();
    }

  emit dataChanged(index(position,0,parent),index(position+rows-1,columnCount(),parent));
  return true;
  }

bool TSheduleModel::hasChildren(const QModelIndex &parent) const
  {
  QList<TSheduleModel::Types> expandable={TSheduleModel::DateType,TSheduleModel::DoctorType};
  if (expandable.contains((TSheduleModel::Types)index(parent.row(),TSheduleModel::Type,parent.parent()).data().toUInt())==true)
    return true;

  return QAbstractItemModel::hasChildren(parent);
  }

void TSheduleModel::clear(const QModelIndex &parent)
  {
  removeRows(0,rowCount(parent),parent);
  }

TSheduleModel::ItemInfo *TSheduleModel::root()
  {
  return &m_root;
  }

QModelIndex TSheduleModel::rootIndex() const
  {
  return m_rootIndex;
  }

QString TSheduleModel::titleByType(TSheduleModel::Types type)
  {
  switch (type)
    {
    case TSheduleModel::DateType:
     return tr("Date");

    case TSheduleModel::DoctorType:
      return tr("Doctor");

    case TSheduleModel::TimeType:
      return tr("Appointment");
    }

  return QString();
  }

QVariantList TSheduleModel::fullPath(QModelIndex current)
  {
  QVariantList data;
  while (current.isValid()==true && current!=m_rootIndex)
    {
    QVariant value;
    switch ((TSheduleModel::Types)index(current.row(),TSheduleModel::Type,current.parent()).data().toUInt())
      {
      case TSheduleModel::DateType:
        {
        value=index(current.row(),TSheduleModel::Date,current.parent()).data().toDate();
        break;
        }
      case TSheduleModel::DoctorType:
        {
        value=index(current.row(),TSheduleModel::DoctorId,current.parent()).data().toUInt();
        break;
        }
      case TSheduleModel::TimeType:
        {
        value=index(current.row(),TSheduleModel::Time,current.parent()).data().toTime();
        break;
        }
      }

    data.prepend(value);
    current=current.parent();
    }

  return data;
  }

QModelIndex TSheduleModel::findMatch(const QVariantList &data)
  {
  if (data.size()==0)
    return QModelIndex();

  // снчала найдем с помощью банального match сопадение по Дате
  QModelIndexList dates=match(index(0,TSheduleModel::Date,m_rootIndex),Qt::DisplayRole,data.at(0),1,Qt::MatchExactly); // найти дату
  if (dates.size()==0)
    return QModelIndex();

  QModelIndex dateIndex=dates.first();
  if (data.size()==1)
    return index(dateIndex.row(),TSheduleModel::Title,m_rootIndex);

  quint16 datesSize=rowCount(dateIndex);
  for (quint16 i=0;i<datesSize;i++)
    {
    QModelIndex doctorIndex=index(i,TSheduleModel::DoctorId,dateIndex);
    if (doctorIndex.data()!=data.at(1))
      continue;

    if (data.size()==2)
      return index(doctorIndex.row(),TSheduleModel::Title,dateIndex);

    quint16 doctorSize=rowCount(doctorIndex);
    for (quint16 j=0;j<doctorSize;j++)
      {
      QModelIndex timeIndex=index(j,TSheduleModel::Time,doctorIndex);
      if (timeIndex.data()!=data.at(2))
        continue;

      return index(timeIndex.row(),TSheduleModel::Title,doctorIndex);
      }

    return QModelIndex();//index(doctorIndex.row(),TSheduleModel::Title,dateIndex);
    }

  return QModelIndex();//index(dateIndex.row(),TSheduleModel::Title,m_rootIndex);
  }
