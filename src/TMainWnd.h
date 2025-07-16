#pragma once

#include "ui_TMainWnd.h"

#include <QSqlRecord>

class QNetworkAccessManager;
class QProgressDialog;
class TDataModule;

class TPetcoreApiWorker;
class TDadataApiWorker;
class TQwenApiWorker;
class TGigachatApiWorker;

class TSqlTableModel;

class TDoctorsModel;
class TClientsModel;
class TPetsModel;

class TRoutinesModel;

class TSpeciesModel;
class TSubSpeciesModel;
class TMarkingModel;
class TStatusesModel;

class TProfilesModel;
class TActionsModel;
class TPetagesModel;

class TRemindersModel;

class TDataDelegate;
class TRemindersDelegate;

class TApptsModel;

class TSheduleModel;
class TSheduleDelegate;

class TModelDlg;
class QSplashScreen;

class TMainWnd : public QMainWindow, private Ui::TMainWnd
  {
    Q_OBJECT
  public:
    // страницы в pagesStacked - либо работа с данными, либо с LLM, либо с Расписанием (?)
    enum StackedPages : quint8
      {
      ShedulePage=0,
      DataPage,
      LlmPage
      };
    Q_ENUM(StackedPages)

    // вкладки в m_adminTabBar - Визиты, Клиенты, Животные, LLM, со тсраницами (выше) пересекается, но не один-в-один
    enum PageTabs : qint8
      {
      UndefinedTab=-1,
      SheduleTab=0,
      PetsTab,
      ClientsTab,
      DoctorsTab,
      LlmTab
      };
    Q_ENUM(PageTabs)

    // страницы в filterStacked - либо фильтр для животных и визитов, либо для докторов и расписания
    enum FilterPages : quint8
      {
      PetsFilter=0,
      DoctorsFilter
      };
    Q_ENUM(FilterPages)

    enum AddonPages : quint8
      {
      SearchAddon=0,
      PeriodAddon
      };
    Q_ENUM(AddonPages)

    enum SlotTypes : quint8
      {
      AllTimeSlots=0,
      ActualTimeSlots,
      ExpiredTimeSlots
      };
    Q_ENUM(SlotTypes)

    struct ExportData
      {
      ExportData(TSqlTableModel *model=nullptr, const QString &caption=QString(), const QStringList &headers={}, const QStringList &fields={})
        {
        this->model=model;
        this->caption=caption;
        this->headers=headers;
        this->fields=fields;
        }

      TSqlTableModel *model;

      QString caption;
      QStringList headers;
      QStringList fields;
      };

    explicit TMainWnd(QWidget *parent = 0);

  protected:
    QAction *separatorActn();
    void writeSettings();
    void readSettings();

    void closeEvent(QCloseEvent *event);

    void createWaitDlg(const QString &text);
    void destroyWaitDlg();

    void createSplashScreen(const QString &text);
    void destroySplashScreen();

    void adjustDataActions();
    void setupDataIndex(const QModelIndex &index);

    QString pageTabText(TMainWnd::PageTabs tab) const;
    TModelDlg *dialogByModel(TSqlTableModel *model, const QSqlRecord &record);

    void adjustSheduleActions();
    void setupSheduleIndex(const QModelIndex &index, quint32 scroll);

    void buildDateBranch(const QModelIndex &dateIndex);
    void buildDoctorBranch(const QModelIndex &doctorIndex);

    QMap<QDate, QList<quint32>> solveExpanded();

    void buildShedule();
    void expandShedule(const QMap <QDate, QList<quint32>> &expanded);

    QModelIndex lastApptIndex();

    inline QHash <QString,QVariant> recordToHash(const QSqlRecord &record);
    inline QSqlRecord hashToRecord(const QSqlRecord &pattern, const QHash<QString,QVariant> &dataHash);

    quint32 createTimeSlot(const QModelIndex &doctorIndex, qint32 timeRow, const QTime &time, bool available);
    void updateTimeSlot(const QModelIndex &doctorIndex, quint32 timeRow, qint32 apptRow);
    void removeTimeSlot(const QModelIndex &doctorIndex, quint32 timeRow);

    bool createApptsFilter();
    TMainWnd::ExportData getExportData();

  protected slots:
    void aboutToQuit();
    void about();

    void pageTabChanged(int index);

    void dataSelectionChanged(const QItemSelection &selected, const QItemSelection &deselected);
    void dataTableActivated(const QModelIndex &index);

    void sheduleSelectionChanged(const QItemSelection &selected, const QItemSelection &deselected);
    void sheduleTreeActivated(const QModelIndex &index);
    void showSheduleAppts(QMap<QDate, QList<quint32>> expanded);

    void renewTimeSlots();

    void appendAppt();
    void makeAppt();
    void editAppt();

    void removeAppt();
    void cutAppt();
    void pasteAppt();
    void startAppt();
    void finishAppt();

    void appendData(TSqlTableModel *dataModel);
    void editData(TSqlTableModel *dataModel, qint32 row);
    void removeData(TSqlTableModel *dataModel, qint32 row);
    void updateData(TSqlTableModel *dataModel, quint32 id);

    void remindersSelectionChanged(const QItemSelection &selected, const QItemSelection &deselected);
    void remindersTableActivated(const QModelIndex &index);

    void appendReminder();
    void removeReminder();

    void clearDialog();
    void saveDialog();
    void findInDialog();

    void findInField();

    void shedulePeriodChanged(const QDate &date);
    void timeSlotsChanged(int index);

    void createPetsFilter(int index);
    void petsFilterGroupClicked(bool checked);

    void createDoctorsFilter(int index);
    void doctorsFilterGroupClicked(bool checked);

    void exportToCsv();
    void exportToXml();

    void makeSettings();

    void checkArchiveNeeded();
    void makeArchive();

    void processAuthorization();
    void checkToken();
    void sendTelegram();
    void loadDictionaries();

    void sendLlmQuery();
    void llmAnswerReceived(const QJsonObject &response);

  private:
    QProgressDialog *m_waitDlg;
    // SplashScreen для некоторых долгих операций, например синхронизация или вытягивание справочников
    QSplashScreen *m_splashScreen;

    TDataModule *m_dataModule;

    TDoctorsModel *m_doctorsModel;
    TClientsModel *m_clientsModel;
    TPetsModel *m_petsModel;

    TRoutinesModel *m_routinesModel;

    TSpeciesModel *m_speciesModel;
    TMarkingModel *m_markingModel;
    TStatusesModel *m_statusesModel;

    TProfilesModel *m_profilesModel;
    TActionsModel *m_actionsModel;
    TPetagesModel *m_petagesModel;

    TDataDelegate *m_dataDelegate;

    TRemindersModel *m_remindersModel;
    TRemindersDelegate *m_remindersDelegate;

    TApptsModel *m_apptsModel;

    TSheduleModel *m_sheduleModel;
    TSheduleDelegate *m_sheduleDelegate;

    QTabBar *m_pagesTabBar;

    QToolBar *m_sheduleToolBar; // ToolBar для Расписания
    QToolBar *m_dataToolBar;    // ToolBar для основных данных, где модель динамически просто меняется (Визиты, Питомцы, Клиенты, Доктора)

    TMainWnd::PageTabs m_lastActiveTab; // для хранения последнего индекса таба
    bool m_dataChanged; // флаг, что поменялись данные в m_petsModel, m_clientsModel или m_doctorsModel, надо перестроить m_sheduleModel
    QSqlRecord m_memApptRecord; // record из m_apptsModel, таскаемая между cutAppt() и pasteApp()

    QTimer *m_archiveTimer;


    // про работу с API
    TPetcoreApiWorker *m_petcoreApiWorker;
    TDadataApiWorker *m_dadataApiWorker;
    TQwenApiWorker *m_qwenApiWorker;
    TGigachatApiWorker *m_gigachatApiWorker;
  };
