#pragma once

#include <QSqlDatabase>
#include <QObject>
#include <QSqlRecord>

#include <QStandardPaths>
#include <QHash>

class TSqlTableModel;

class TStatusesModel;
class TMarkingModel;
class TProfilesModel;

class TDegreesModel;
class TRanksModel;
class TPetagesModel;
class TActionsModel;

class TSpeciesModel;
class TSubspeciesModel;
class TBreedsModel;

class TDoctorsModel;
class TClientsModel;
class TPetsModel;

class TRemindersModel;
class TRoutinesModel;
class TApptsModel;

class TDataModule : public QObject
  {
    Q_OBJECT
  public:
    explicit TDataModule(QObject *parent=nullptr);
    static TDataModule *instance();

    bool openBase(const QString &baseName=QStandardPaths::writableLocation(QStandardPaths::AppDataLocation)+"/petcore.sqt");//":memory:"
    void closeBase();

    QSqlDatabase database() const;
    QString baseName() const;

    TSqlTableModel *tableModel(const QString &table);
    QSqlRecord tableRecord(const QString &table) const;
    qint8 fieldIndex(const QString &table, const QString &field=QString("header_field"));

    qint32 lastAutoInc(const QString &tableName);

    void updateModel(TSqlTableModel *model);
    void updateModel(const QString &table);

  protected:
    void correctStructure();

  private:
    static TDataModule *m_instance;
    QSqlDatabase m_petcoreBase;

    TStatusesModel *m_statusesModel;          // статусы
    TMarkingModel *m_markingModel;            // типы маркировки
    TProfilesModel *m_profilesModel;          // профили докторов

    TDegreesModel *m_degreesModel;            // ученые степени
    TRanksModel *m_ranksModel;                // ученые звания
    TPetagesModel *m_petagesModel;            // возрастные группы животных
    TActionsModel *m_actionsModel;            // специализации докторов, оно же используется в Целях посещений

    TSpeciesModel *m_speciesModel;            // виды
    TSubspeciesModel *m_subspeciesModel;      // подвиды, не нужны?
    TBreedsModel *m_breedsModel;              // породы

    TDoctorsModel *m_doctorsModel;            // доктора
    TClientsModel *m_clientsModel;            // клиенты
    TPetsModel *m_petsModel;                  // животные

    TRemindersModel *m_remindersModel;        // напоминания
    TRoutinesModel *m_routinesModel;          // расписания врачей
    TApptsModel *m_apptsModel;                // назначенные приемы врачей (appts - сокращение от appointments)

    QHash <QString, TSqlTableModel*> m_tableHash;
  };
