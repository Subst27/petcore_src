#pragma once

#include <QAbstractItemModel>
#include <QSqlDatabase>
#include <QSqlRecord>
#include <QDate>
#include <QTime>

#include <QDataStream>

class TSheduleModel : public QAbstractItemModel
  {
    Q_OBJECT

  private:
    struct ItemInfo
      {
      public:
        ItemInfo(struct ItemInfo *parent=nullptr,const QVector <struct ItemInfo*> &children={}, const QVector <QVariant> &data={})
          {
          this->parent=parent;
          this->children=children;
          this->data=data;
          }

        struct ItemInfo *parent;
        QVector <struct ItemInfo*> children;
        QVector <QVariant> data;
      };

  public:
    enum Columns : quint8
      {
      Title,          // отображаемые данные, что именно отображать - определяется в делегате из DataRecord
      Date,           // собственные данные, задается из ГУИ
      DoctorId,       // из TRoutinesModel, но оставил и тут для удобства и быстрого доступа
      Time,           // собственные данные, считается при построении, исходя из данных TRoutinesModel
      // хотелось бы хранить QModelIndex модели с данными, но слишком скользко - фильтр на модели и Индекс уедет
      // надо хранить QHash, при запихивании данных перекинуть из QSqlRecord в QHash
      DataRecord,     // запись из модели, TRooutinesModel (для type=DoctorType) или TApptsModel (для type=TimeType)
      Type,           // тип узла, можно вычислить по уровню иерархии, но отсавил для удобства
      };
    Q_ENUM(Columns)

    // типы узлов
    enum Types : quint8
      {
      DateType,     // дата приема
      DoctorType,   // доктор
      TimeType      // время приема
      };
    Q_ENUM(Types)

    explicit TSheduleModel(QObject *parent=nullptr);
    ~TSheduleModel();

    void setPeriod(const QDate &fromDate, const QDate &toDate);
    QPair<QDate, QDate> period() const;

    QVariantList record(const QModelIndex &parent, quint16 row, int role=Qt::DisplayRole) const;

    /*QStringList fields() const;
    qint8 fieldIndex(const QString &fieldName) const;*/

    QVariant data(const QModelIndex &index,int role=Qt::DisplayRole) const override;
    bool setData(const QModelIndex &index,const QVariant &value,int role=Qt::EditRole) override;

    QVariant headerData(int section,Qt::Orientation orientation,int role = Qt::DisplayRole) const override;
    bool setHeaderData(int section,Qt::Orientation orientation,const QVariant &value,int role=Qt::EditRole) override;

    QModelIndex index(int row,int column,const QModelIndex &parent=QModelIndex()) const override;
    QModelIndex parent(const QModelIndex &index) const override;

    int rowCount(const QModelIndex &parent=QModelIndex()) const override;
    int columnCount(const QModelIndex &parent=QModelIndex()) const override;

    int columnPosition(const QString &name);

    Qt::ItemFlags flags(const QModelIndex &index) const override;

    bool insertColumns(int position,int columns,const QModelIndex &parent=QModelIndex()) override;
    bool removeColumns(int position,int columns,const QModelIndex &parent=QModelIndex()) override;

    bool insertRows(int position,int rows,const QModelIndex &parent=QModelIndex()) override;
    bool removeRows(int position,int rows,const QModelIndex &parent=QModelIndex()) override;

    bool hasChildren(const QModelIndex &parent) const override;

    void clear(const QModelIndex &parent);

    ItemInfo *root();
    QModelIndex rootIndex() const;

    static QString titleByType(TSheduleModel::Types type);
    // метод - подняться от выбранного индекса на самый верх и получить полный путь дерева
    QVariantList fullPath(QModelIndex current);
    // обратная процедура - получить индекс по пути {Дата, Доктор, Время}
    QModelIndex findMatch(const QVariantList &data);

    /*friend QDataStream &operator <<(QDataStream &stream,const TSheduleModel::ApptInfo &apptInfo) {
      stream << apptInfo.date << apptInfo.time << apptInfo.doctorId << apptInfo.actionId << apptInfo.clientId << apptInfo.petId;
      return stream;
      }

    friend QDataStream &operator >>(QDataStream &stream,TSheduleModel::ApptInfo &apptInfo) {
      stream >> apptInfo.date >> apptInfo.time >> apptInfo.doctorId >> apptInfo.actionId >> apptInfo.clientId >> apptInfo.petId;
      return stream;
      }*/

  protected:

  private:
    ItemInfo m_root;
    QModelIndex m_rootIndex;

    QStringList m_headers;
    QStringList m_fields;

    QDate m_fromDate;
    QDate m_toDate;
  };
