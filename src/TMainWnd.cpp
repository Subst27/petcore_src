#include "TMainWnd.h"

#include "TSettings.h"
#include "QAesEncryption.h"

#include "TPetcoreApiWorker.h"
#include "TDadataApiWorker.h"
#include "TQwenApiWorker.h"
#include "TGigachatApiWorker.h"

#include "TDataModule.h"

#include "TDoctorsModel.h"
#include "TClientsModel.h"
#include "TPetsModel.h"

#include "TSpeciesModel.h"
#include "TBreedsModel.h"

#include "TMarkingModel.h"
#include "TStatusesModel.h"

#include "TRoutinesModel.h"
#include "TRoutinesFilterModel.h"

#include "TProfilesModel.h"
#include "TActionsModel.h"
#include "TPetagesModel.h"

#include "TDegreesModel.h"
#include "TRanksModel.h"

#include "TDataDelegate.h"

#include "TTasksModel.h"
#include "TRemindersModel.h"
#include "TTasksDelegate.h"

#include "TApptsModel.h"
#include "TApptsFilterModel.h"

#include "TSheduleModel.h"
#include "TSheduleDelegate.h"

#include "TLoginDlg.h"
#include "TAboutDlg.h"

#include "TApptDlg.h"
#include "TDoctorDlg.h"
#include "TClientDlg.h"
#include "TPetDlg.h"

#include "TTaskDlg.h"
#include "TSettingsDlg.h"

#include <QDebug>

#include <QMessageBox>
#include <QCloseEvent>
#include <QToolBar>

#include <QStandardPaths>
#include <QFileInfo>
#include <QMetaEnum>
#include <QFileDialog>

#include <QProgressDialog>
#include <QSplashScreen>
#include <QInputDialog>
#include <QScrollBar>

#include <QTimer>

#include <QJsonObject>
#include <QJsonArray>

#include <QSqlQuery>
#include <QSqlField>

TMainWnd::TMainWnd(QWidget *parent) :
  QMainWindow(parent),
  m_waitDlg(nullptr),
  m_splashScreen(nullptr),
  m_lastActiveTab(TMainWnd::UndefinedTab),
  m_dataChanged(false),
  m_archiveTimer(new QTimer(this))
  {
  setupUi(this);

  qApp->setApplicationName("PetCore");
  qApp->setApplicationVersion(PETCORE_VERSION);
  setWindowTitle(tr("%1 [v.%2]").arg(qApp->applicationName(),qApp->applicationVersion()));

  connect(qApp,&QApplication::aboutToQuit,this,&TMainWnd::aboutToQuit);

  connect(exitActn,&QAction::triggered,this,&TMainWnd::close);
  connect(aboutQtActn,&QAction::triggered,qApp,&QApplication::aboutQt);
  connect(aboutActn,&QAction::triggered,this,&TMainWnd::about);

  // NOTE: это для проверки словарей, по-хорошему убрать потом надо
  connect(dictionaryActn,&QAction::triggered, this, &TMainWnd::loadDictionaries);

  // про диалог с LLM
  dialogEdit->setMaximumBlockCount(1024); // ограничить кол-во хранимых блоков (де-факто строк) в диалоге
  connect(clearDialogBtn, &QPushButton::clicked, this, &TMainWnd::clearDialog);
  connect(saveDialogBtn, &QPushButton::clicked, this, &TMainWnd::saveDialog);

  connect(findTextBtn,&QPushButton::clicked,this,&TMainWnd::findInDialog);
  connect(searchTextEdit,&QLineEdit::returnPressed,this,&TMainWnd::findInDialog);

  connect(askBtn,&QPushButton::clicked,this,&TMainWnd::sendLlmQuery);
  connect(questionEdit,&QLineEdit::returnPressed,this,&TMainWnd::sendLlmQuery);

  // про экспорт
  connect(exportCsvBtn,&QPushButton::clicked,this,&TMainWnd::exportToCsv);
  connect(exportXmlBtn,&QPushButton::clicked,this,&TMainWnd::exportToXml);

  // про настройки и архивацию
  connect(settingsActn,&QAction::triggered,this,&TMainWnd::makeSettings);
  connect(archiveActn,&QAction::triggered,this,&TMainWnd::makeArchive);

  // про панельку поиска
  connect(searchDataEdit,&QLineEdit::returnPressed,this,&TMainWnd::findInField);
  connect(findDataBtn,&QPushButton::clicked,this,&TMainWnd::findInField);

  searchModeCombo->addItem(tr("Match Exactly"),Qt::MatchFixedString);
  searchModeCombo->addItem(tr("Starts With..."),Qt::MatchStartsWith);
  searchModeCombo->addItem(tr("Contains..."),Qt::MatchContains);
  searchModeCombo->setCurrentIndex(0);

  // определить имя файлов конфигурации и БД и создать их из шаблона при необходимости
  QString configName;
  QString baseName;
  if (qApp->arguments().contains("-develop")==true)
    {
    configName=qApp->applicationDirPath()+"/petcore.xml";
    baseName=qApp->applicationDirPath()+"/dbase/petcore.sqt";
    }
  else
    {
    configName=QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation)+"/petcore.xml";
    baseName=QStandardPaths::writableLocation(QStandardPaths::AppDataLocation)+"/petcore.sqt";
    }

  // скоипровать файлы из шаблона(ресурсы) при необходимости
  QStringList names={configName, baseName};
  foreach (const QString name, names)
    {
    if (QFile::exists(name)==true)
      continue;

    QFileInfo info(name);
    QDir dir;
    if (dir.exists(info.absolutePath())==false)
      dir.mkpath(info.absolutePath());

    QFile::copy(":/templates/templates/"+info.fileName(),name);
    QFile::setPermissions(name,QFile::permissions(name) | QFile::WriteOwner | QFile::WriteUser | QFile::WriteGroup | QFile::WriteOther |
                                                          QFile::ReadOwner | QFile::ReadUser | QFile::ReadGroup | QFile::ReadOther);
    // время последнего архива БД - время первого запуска программы
    if (name==baseName)
      TSettings().setXmlValue("archiving/date_time","",QDateTime::currentDateTime().toSecsSinceEpoch());
    }

  // про БД и модели
  m_dataModule=new TDataModule(this);
  if (m_dataModule->openBase(baseName)==false)
    {
    QMessageBox::critical(this, tr("Error"), tr("Can't open database file:\nor database file the invalid database:\n%1").arg(baseName),QMessageBox::Ok);
    qApp->quit();
    return;
    }

  // для Приемов - смена дат от и до
  connect(startDateEdit,&QDateEdit::dateChanged,this,&TMainWnd::shedulePeriodChanged);
  connect(endDateEdit,&QDateEdit::dateChanged,this,&TMainWnd::shedulePeriodChanged);
  // и смена типа отображаемых тайм-слотов
  timeslotCombo->addItem(tr("All"), TMainWnd::AllTimeSlots);
  timeslotCombo->addItem(tr("Actual"), TMainWnd::ActualTimeSlots);
  timeslotCombo->addItem(tr("Expired"), TMainWnd::ExpiredTimeSlots);

  connect(timeslotCombo,qOverload<int>(&TComboBox::activated),this,&TMainWnd::timeSlotsChanged);

  // модели
  m_doctorsModel=qobject_cast<TDoctorsModel*>(m_dataModule->tableModel("doctors"));
  connect(m_doctorsModel,&TClientsModel::dataChanged,this,[this]() {
    m_dataChanged=true;
    });

  m_clientsModel=qobject_cast<TClientsModel*>(m_dataModule->tableModel("clients"));
  connect(m_clientsModel,&TClientsModel::dataChanged,this,[this]() {
    m_dataChanged=true;
    });

  m_petsModel=qobject_cast<TPetsModel*>(m_dataModule->tableModel("pets"));
  connect(m_petsModel,&TClientsModel::dataChanged,this,[this]() {
    m_dataChanged=true;
    });

  m_dataDelegate=new TDataDelegate(this);
  dataTable->setItemDelegate(m_dataDelegate);

  // расписания докторов, тоже нужны будут для быстрого доступа
  m_routinesModel=qobject_cast<TRoutinesModel*>(m_dataModule->tableModel("routines"));

  // про панельку с фильтрами Питомцев
  connect(petsFilterGroup,&QGroupBox::clicked,this,&TMainWnd::petsFilterGroupClicked);

  m_speciesModel=qobject_cast<TSpeciesModel*>(m_dataModule->tableModel("species"));
  speciesCombo->setModel(m_speciesModel,{"species","forbidden"},"id");
  connect(speciesCombo,qOverload<int>(&TComboBox::activated),this,&TMainWnd::createPetsFilter);

  m_markingModel=qobject_cast<TMarkingModel*>(m_dataModule->tableModel("marking"));
  markingCombo->setModel(m_markingModel,{"marking"},"id");
  connect(markingCombo,qOverload<int>(&TComboBox::activated),this,&TMainWnd::createPetsFilter);

  m_statusesModel=qobject_cast<TStatusesModel*>(m_dataModule->tableModel("statuses"));
  statusCombo->setModel(m_statusesModel,{"status"},"id");
  connect(statusCombo,qOverload<int>(&TComboBox::activated),this,&TMainWnd::createPetsFilter);

  // про панельку с фильтрами Докторов
  connect(doctorsFilterGroup,&QGroupBox::clicked,this,&TMainWnd::doctorsFilterGroupClicked);

  m_profilesModel=qobject_cast<TProfilesModel*>(m_dataModule->tableModel("profiles"));
  profileCombo->setModel(m_profilesModel,{"profile"},"id");
  connect(profileCombo,qOverload<int>(&TComboBox::activated),this,&TMainWnd::createDoctorsFilter);

  m_actionsModel=qobject_cast<TActionsModel*>(m_dataModule->tableModel("actions"));
  actionCombo->setModel(m_actionsModel,{"action"},"id");
  connect(actionCombo,qOverload<int>(&TComboBox::activated),this,&TMainWnd::createDoctorsFilter);

  m_petagesModel=qobject_cast<TPetagesModel*>(m_dataModule->tableModel("petages"));
  petageCombo->setModel(m_petagesModel,{"pet_age"},"id");
  connect(petageCombo,qOverload<int>(&TComboBox::activated),this,&TMainWnd::createDoctorsFilter);

  /* про расписание, записи на прием */
  // ToolBar для Расписания
  m_sheduleToolBar=new QToolBar(tr("Shedule tools"),this);
  m_sheduleToolBar->setFloatable(false);
  m_sheduleToolBar->setMovable(false);
  m_sheduleToolBar->setContextMenuPolicy(Qt::PreventContextMenu);
  m_sheduleToolBar->setIconSize(QSize(20,20));
  sheduleLayout->insertWidget(0,m_sheduleToolBar);

  // добавить Actions в sheduleTree и sheduleToolBar
  QList<QAction*> actions={appendApptActn,editApptActn,removeApptActn,separatorActn(),cutApptActn,pasteApptActn,separatorActn(),startApptActn,finishApptActn};
  m_sheduleToolBar->addActions(actions);

  actions.append({separatorActn(),telegramActn});
  sheduleTree->addActions(actions);

  m_apptsModel=qobject_cast<TApptsModel*>(m_dataModule->tableModel("appts"));

  m_sheduleModel=new TSheduleModel(this);
  sheduleTree->setModel(m_sheduleModel);
  sheduleTree->setRootIndex(m_sheduleModel->rootIndex());

  m_sheduleDelegate=new TSheduleDelegate(this);
  sheduleTree->setItemDelegate(m_sheduleDelegate);
  sheduleTree->header()->setDefaultAlignment(Qt::AlignVCenter | Qt::AlignHCenter);

  connect(appendApptActn,&QAction::triggered,this,&TMainWnd::appendAppt);
  connect(editApptActn,&QAction::triggered,this,&TMainWnd::editAppt);
  connect(removeApptActn,&QAction::triggered,this,&TMainWnd::removeAppt);
  connect(cutApptActn,&QAction::triggered,this,&TMainWnd::cutAppt);
  connect(pasteApptActn,&QAction::triggered,this,&TMainWnd::pasteAppt);
  connect(startApptActn,&QAction::triggered,this,&TMainWnd::startAppt);
  connect(finishApptActn,&QAction::triggered,this,&TMainWnd::finishAppt);

  connect(sheduleTree,&QTreeView::expanded,this,&TMainWnd::sheduleTreeExpanded);
  connect(sheduleTree,&QTreeView::activated,this,&TMainWnd::sheduleTreeActivated);
  connect(sheduleTree->selectionModel(),&QItemSelectionModel::selectionChanged,this,&TMainWnd::sheduleSelectionChanged);

  /* про базовые данные, которые в DataPage (Питомцы, Клиенты, Доктора) */
  // ToolBar для основных данных (на странице DataPage: Питомцы, Клиенты, Доктора)
  m_dataToolBar=new QToolBar(tr("Data tools"),this);
  m_dataToolBar->setFloatable(false);
  m_dataToolBar->setMovable(false);
  m_dataToolBar->setContextMenuPolicy(Qt::PreventContextMenu);
  m_dataToolBar->setIconSize(QSize(20,20));
  dataLayout->insertWidget(0,m_dataToolBar);

  // добавить Actions в dataTable и dataToolBar
  actions={appendDataActn,editDataActn,removeDataActn};
  m_dataToolBar->addActions(actions);

  actions.append({separatorActn(),appendRemActn,makeApptActn,separatorActn(),telegramActn});
  dataTable->addActions(actions);

  connect(appendDataActn,&QAction::triggered,this,[this]() {
    appendData(nullptr);
    });

  connect(editDataActn,&QAction::triggered,this,[this]() {
    editData(nullptr, -1);
    });

  connect(removeDataActn,&QAction::triggered,this,[this]() {
    removeData(nullptr, -1);
    });

  connect(makeApptActn,&QAction::triggered,this,&TMainWnd::makeAppt);
  connect(dataTable,&QTableView::activated,this,&TMainWnd::dataTableActivated);
  // QItemSelectionModel::selectionChanged цепляется в pagesTabChanged, потому что тут пока не назначена модель для dataTable

  // про телегу, общее для sheduleTree & dataTable
  connect(telegramActn,&QAction::triggered,this,&TMainWnd::sendTelegram);

  /* про Частые задачи (Срочные задачи и Напоминания) */
  // про срочные дела
  m_tasksDelegate=new TTasksDelegate(this);

  m_tasksModel=qobject_cast<TTasksModel*>(m_dataModule->tableModel("tasks"));
  tasksTable->setModel(m_tasksModel);
  tasksTable->setItemDelegate(m_tasksDelegate);

  // ToolBar для Срочных задач
  m_tasksToolBar=new QToolBar(tr("Tasks tools"),this);
  m_tasksToolBar->setFloatable(false);
  m_tasksToolBar->setMovable(false);
  m_tasksToolBar->setContextMenuPolicy(Qt::PreventContextMenu);
  m_tasksToolBar->setIconSize(QSize(20,20));
  urgentLayout->insertWidget(1,m_tasksToolBar);

  // добавить Actions в tasksTable и tasksToolBar
  actions={appendTaskActn,editTaskActn,removeTaskActn,separatorActn(),completeTaskActn};
  m_tasksToolBar->addActions(actions);
  tasksTable->addActions(actions);

  connect(appendTaskActn,&QAction::triggered,this,&TMainWnd::appendTask);
  connect(editTaskActn,&QAction::triggered,this,&TMainWnd::editTask);
  connect(removeTaskActn,&QAction::triggered,this,&TMainWnd::removeTask);
  connect(completeTaskActn,&QAction::triggered,this,&TMainWnd::completeTask);

  connect(tasksTable,&QTreeView::activated,this,&TMainWnd::tasksTableActivated);
  connect(tasksTable->selectionModel(),&QItemSelectionModel::selectionChanged,this,&TMainWnd::tasksSelectionChanged);
  tasksSelectionChanged(QItemSelection(),QItemSelection());

  // про напоминалки
  m_remindersModel=qobject_cast<TRemindersModel*>(m_dataModule->tableModel("reminders"));
  remindersTable->setModel(m_remindersModel);
  remindersTable->setItemDelegate(m_tasksDelegate);

  remindersTable->addAction(removeRemActn);

  connect(appendRemActn,&QAction::triggered,this,&TMainWnd::appendReminder);
  connect(removeRemActn,&QAction::triggered,this,&TMainWnd::removeReminder);

  connect(remindersTable,&QTableView::activated,this,&TMainWnd::remindersTableActivated);
  connect(remindersTable->selectionModel(),&QItemSelectionModel::selectionChanged,this,&TMainWnd::remindersSelectionChanged);
  remindersSelectionChanged(QItemSelection(),QItemSelection());

  // про TabBar, где вкладки выбираются (Питомцы, Клиенты, Доктора...)
  m_pagesTabBar=new QTabBar(this);
  m_pagesTabBar->setMovable(false);
  m_pagesTabBar->setTabsClosable(false);
  m_pagesTabBar->setDocumentMode(true);
  m_pagesTabBar->setExpanding(false);
  centralLayout->insertWidget(0,m_pagesTabBar);

  m_pagesTabBar->addTab(QIcon(":/images/appts"),tr("Shedule"));
  m_pagesTabBar->addTab(QIcon(":/images/pets"),tr("Pets"));
  m_pagesTabBar->addTab(QIcon(":/images/clients"), tr("Clients"));
  m_pagesTabBar->addTab(QIcon(":/images/doctors"), tr("Doctors"));
  m_pagesTabBar->addTab(QIcon(":/images/llm_vet"), tr("Online veterinarian"));

  connect(m_pagesTabBar,&QTabBar::currentChanged,this,&TMainWnd::pageTabChanged);

  resizeDocks({tasksDock},{width()/4},Qt::Horizontal);
  readSettings();

  // про отчеты
  m_report=new LimeReport::ReportEngine(this);
  m_report->setShowProgressDialog(true);
  m_report->setPreviewWindowIcon(QIcon(QPixmap(":/images/petcore")));

  // Dadata API
  m_dadataApiWorker=new TDadataApiWorker(this);
  m_dadataApiWorker->initialize("dadata_api");

  // QWEN API
  m_qwenApiWorker=new TQwenApiWorker(this);
  m_qwenApiWorker->initialize("qwen_api");
  connect(m_qwenApiWorker,&TQwenApiWorker::answerReceived,this,&TMainWnd::llmAnswerReceived);

  // GigaChat API
  m_gigachatApiWorker=new TGigachatApiWorker(this);
  m_gigachatApiWorker->initialize("gigachat_api");
  connect(m_gigachatApiWorker,&TGigachatApiWorker::answerReceived,this,&TMainWnd::llmAnswerReceived);

  // Petcore API
  m_petcoreApiWorker=new TPetcoreApiWorker(this);
  m_petcoreApiWorker->initialize("petcore_api");

  // проверить токен и ID клиники, если кто-то из них пуст -> диалог логина через 300 мс
  TSettings settings;
  QByteArray token=settings.getXmlValue("petcore_api/access_token","",QByteArray()).toByteArray();
  quint32 clinicId=settings.getXmlValue("clinic/id","",0).toUInt();
  if (token.isEmpty()==true || clinicId==0)
    {
    QTimer::singleShot(300,this,&TMainWnd::processAuthorization);
    return;
    }

  QString clinicTitle=settings.getXmlValue("clinic/title","","").toString();
  setWindowTitle(windowTitle()+QString(" [%1]").arg(clinicTitle));

  //checkToken();
  QTimer::singleShot(300,this,&TMainWnd::checkToken);
  renewTimeSlots();

  m_archiveTimer->setTimerType(Qt::CoarseTimer);
  m_archiveTimer->setInterval(1800000);
  connect(m_archiveTimer,&QTimer::timeout,this,&TMainWnd::checkArchiveNeeded);

  checkArchiveNeeded();
  m_archiveTimer->start();
  }

QAction *TMainWnd::separatorActn()
  {
  QAction *separator=new QAction(this);
  separator->setSeparator(true);
  return separator;
  }

void TMainWnd::writeSettings()
  {
  TSettings settings;
  settings.setXmlValue("main_window/state","",saveState());
  settings.setXmlValue("main_window/geometry","",saveGeometry());

  settings.setXmlValue("main_window/page_tab","",m_pagesTabBar->currentIndex());
  settings.setXmlValue("main_window/search_mode","",searchModeCombo->currentIndex());

  // период выборки расписания
  settings.setXmlValue("main_window/shedule_period/start_date","",startDateEdit->date());
  settings.setXmlValue("main_window/shedule_period/end_date","",endDateEdit->date());

  // типы тайм-слотов, которые строить в shedulTree
  settings.setXmlValue("main_window/shedule_timeslots","",timeslotCombo->currentIndex());

  // фильтр животных
  settings.setXmlValue("main_window/pets_filter/species","",speciesCombo->currentIndex());
  settings.setXmlValue("main_window/pets_filter/marking","",markingCombo->currentIndex());
  settings.setXmlValue("main_window/pets_filter/status","",statusCombo->currentIndex());
  settings.setXmlValue("main_window/pets_filter/checked","",petsFilterGroup->isChecked());

  // фильтр докторов
  settings.setXmlValue("main_window/doctors_filter/profile","",profileCombo->currentIndex());
  settings.setXmlValue("main_window/doctors_filter/petage","",petageCombo->currentIndex());
  settings.setXmlValue("main_window/doctors_filter/action","",actionCombo->currentIndex());
  settings.setXmlValue("main_window/doctors_filter/checked","",doctorsFilterGroup->isChecked());

  // хедер текущей таблицы, если в pagesStacked активна DataPage
  if ((TMainWnd::StackedPages)pagesStacked->currentIndex()==TMainWnd::DataPage)
    {
    QString headerStatePath="main_window/data_view/"+pageTabText((TMainWnd::PageTabs)m_pagesTabBar->currentIndex());
    settings.setXmlValue(headerStatePath,"",dataTable->horizontalHeader()->saveState());
    }

  settings.setXmlValue("tasks_dock/task_table","",tasksTable->horizontalHeader()->saveState());
  settings.setXmlValue("tasks_dock/reminder_table","",remindersTable->horizontalHeader()->saveState());
  }

void TMainWnd::readSettings()
  {
  TSettings settings;
  restoreState(settings.getXmlValue("main_window/state","",0).toByteArray());
  restoreGeometry(settings.getXmlValue("main_window/geometry","",0).toByteArray());

  QFont font=settings.getXmlValue("main_window/font","",qApp->font()).value<QFont>();
  setAppFont(font);

  QString theme=settings.getXmlValue("main_window/theme","",qApp->property("system_theme").toString()).toString();
  qApp->setProperty("app_theme",theme);
  setAppTheme(theme);

  searchModeCombo->setCurrentIndex(settings.getXmlValue("main_window/search_mode","",0).toUInt());
  quint8 dataTab=settings.getXmlValue("main_window/page_tab","",0).toUInt();
  if (m_pagesTabBar->currentIndex()==dataTab)
    pageTabChanged(dataTab);
  else
    m_pagesTabBar->setCurrentIndex(dataTab);

  // период раписания
  startDateEdit->setDate(settings.getXmlValue("main_window/shedule_period/start_date","",QDate::currentDate().addDays(-7)).toDate());
  endDateEdit->setDate(settings.getXmlValue("main_window/shedule_period/end_date","",QDate::currentDate()).toDate());

  // типы тайм-слотов, которые строить в shedulTree
  timeslotCombo->setCurrentIndex(settings.getXmlValue("main_window/shedule_timeslots","",0).toUInt());

  // фильтр животных
  bool petsFilterChecked=settings.getXmlValue("main_window/pets_filter/checked","",false).toBool();
  petsFilterGroup->setChecked(petsFilterChecked);
  speciesCombo->setCurrentIndex(settings.getXmlValue("main_window/pets_filter/species","",-1).toInt());
  markingCombo->setCurrentIndex(settings.getXmlValue("main_window/pets_filter/marking","",-1).toInt());
  statusCombo->setCurrentIndex(settings.getXmlValue("main_window/pets_filter/status","",-1).toInt());

  petsFilterGroupClicked(petsFilterChecked);

  // фильтр докторов
  bool doctorsFilterChecked=settings.getXmlValue("main_window/doctors_filter/checked","",false).toBool();
  doctorsFilterGroup->setChecked(doctorsFilterChecked);
  profileCombo->setCurrentIndex(settings.getXmlValue("main_window/doctors_filter/profile","",-1).toInt());
  petageCombo->setCurrentIndex(settings.getXmlValue("main_window/doctors_filter/petage","",-1).toInt());
  actionCombo->setCurrentIndex(settings.getXmlValue("main_window/doctors_filter/action","",-1).toInt());

  doctorsFilterGroupClicked(doctorsFilterChecked);

  // спрятать ненужные столбцы в Расписании, а нужный у нас только Title
  for (quint8 column=0;column<m_sheduleModel->columnCount();column++)
    sheduleTree->setColumnHidden(column, column!=TSheduleModel::Title);

  tasksTable->horizontalHeader()->restoreState(settings.getXmlValue("tasks_dock/task_table","",QByteArray()).toByteArray());
  tasksTable->setColumnHidden(m_tasksModel->fieldIndex("id"),true);
  tasksTable->setColumnHidden(m_tasksModel->fieldIndex("completed"),true);

  remindersTable->horizontalHeader()->restoreState(settings.getXmlValue("tasks_dock/reminder_table","",QByteArray()).toByteArray());
  remindersTable->setColumnHidden(m_remindersModel->fieldIndex("id"),true);
  remindersTable->setColumnHidden(m_remindersModel->fieldIndex("client_id"),true);

  m_tasksDelegate->setReminderDeadline(settings.getXmlValue("tasks_dock/reminder_deadline","",7).toUInt());
  }

void TMainWnd::closeEvent(QCloseEvent *event)
  {
  if (QMessageBox::question(this,tr("Confirmation"),tr("Exit program?"),QMessageBox::Yes | QMessageBox::No)==QMessageBox::Yes)
    event->accept();
  else
    event->ignore();
  }

void TMainWnd::createWaitDlg(const QString &text)
  {
  qApp->setOverrideCursor(QCursor(Qt::WaitCursor));

  m_waitDlg=new QProgressDialog(this);
  m_waitDlg->setModal(true);
  m_waitDlg->setWindowTitle(tr("Please wait"));
  m_waitDlg->setLabelText(text);
  m_waitDlg->setWindowFlags(m_waitDlg->windowFlags() & ~Qt::WindowCloseButtonHint);
  m_waitDlg->setCancelButton(nullptr);
  m_waitDlg->setRange(0,0);
  m_waitDlg->resize(300,m_waitDlg->height());
  m_waitDlg->show();
  }

void TMainWnd::destroyWaitDlg()
  {
  if (m_waitDlg==nullptr)
    return;

  m_waitDlg->close();
  delete m_waitDlg;
  m_waitDlg=nullptr;

  qApp->restoreOverrideCursor();
  }

void TMainWnd::createSplashScreen(const QString &text)
  {
  if (m_splashScreen!=nullptr)
    return;

  QPixmap pixmap(":/images/splash");
  int fontSize=20;

  m_splashScreen=new QSplashScreen(pixmap);
  m_splashScreen->setWindowModality(Qt::ApplicationModal);
  m_splashScreen->setFont(QFont("Courier New",fontSize,QFont::ExtraBold));

  showSplashMessage(text,"");
  m_splashScreen->show();
  }

void TMainWnd::showSplashMessage(const QString &text, const QString &detailed)
  {
  if (m_splashScreen==nullptr)
    return;

  QFontMetrics metrics(m_splashScreen->font());
  QString message=text;
  // заменить последовательности пробелов одним проблелом
  while (message.indexOf("  ")>-1)
    message.replace("  "," ");
  // разбить на слова
  QStringList words=message.split(" ");
  // узнать необходимое количество строк
  QRect singlelineRect=metrics.boundingRect(message);
  QRect multilineRect=metrics.boundingRect(QRect(0,0,m_splashScreen->width()-20,0), Qt::TextWordWrap, message);
  quint8 lineCount=multilineRect.height()/singlelineRect.height();
  // сформировать строки для отображения так, чтобы каждая строка помещалась по ширине
  QStringList lines;
  quint8 wordNumber=0;
  for (quint8 i=0;i<lineCount;i++)
    {
    QString line;
    lines.append(line);
    while (metrics.horizontalAdvance(line)<=m_splashScreen->width()-50 && wordNumber<words.size())
      {
      lines[i]=line; // снчала скопируем line в текущий lines
      line.append(words.at(wordNumber++)+" "); // а потом добавим в line очередное слово из words
      }

    // последнее слово уже было лишним, отматаем wordNumber назад
    wordNumber--;
    lines[i]=lines.at(i).trimmed();
    }

  // последняя строка еще не целиком сформирована, надо поправить
  lines[lineCount-1].append(words[wordNumber].prepend(" "));
  if (detailed.isEmpty()==false)
    lines.append(detailed);

  m_splashScreen->showMessage(/*text*/lines.join("\n"),Qt::AlignHCenter | Qt::AlignTop,QColor("#7C7C7C"));
  }

void TMainWnd::destroySplashScreen()
  {
  if (m_splashScreen==nullptr)
    return;

  m_splashScreen->close();
  delete m_splashScreen;
  m_splashScreen=nullptr;
  }

void TMainWnd::aboutToQuit()
  {
  writeSettings();
  }

void TMainWnd::about()
  {
  TAboutDlg aboutDlg;
  aboutDlg.setWindowTitle(tr("%1 [v.%2]").arg(qApp->applicationName(),qApp->applicationVersion()));
  aboutDlg.exec();
  }

void TMainWnd::setAppFont(const QFont &font)
  {
  QString style=qApp->styleSheet();
  /*if (style.isEmpty()==false)
    qApp->setStyleSheet("");*/
  // именно дважды установить qApp.->setFont(font), возьня связана с применением QStyleSheet
  qApp->setFont(font);
  foreach (QObject *child, children())
    {
    QDockWidget *widget=qobject_cast<QDockWidget*>(child);
    if (widget!=nullptr)
      widget->setFont(font);
    }

  if (style.isEmpty()==false)
    qApp->setStyleSheet(style);

  qApp->setFont(font);

  tasksTable->verticalHeader()->setDefaultSectionSize(qApp->font().pointSize()*2.5);
  remindersTable->verticalHeader()->setDefaultSectionSize(qApp->font().pointSize()*2.5);
  dataTable->verticalHeader()->setDefaultSectionSize(qApp->font().pointSize()*2.5);
  }

void TMainWnd::setAppTheme(const QString &theme)
  {
  QPalette palette=qApp->palette();

  palette.setColor(QPalette::Window,theme=="light" ? QColor("#F0F0F0") : QColor("#202326"));
  palette.setColor(QPalette::WindowText,theme=="light" ? QColor("#000000") : QColor("#FCFCFC"));
  palette.setColor(QPalette::Base,theme=="light" ? QColor("#FFFFFF") : QColor("#141618"));
  palette.setColor(QPalette::AlternateBase,theme=="light" ? QColor("#E9E7E3") : QColor("#1D1F22"));
  palette.setColor(QPalette::ToolTipBase,theme=="light" ? QColor("#FFFFDC") : QColor("#292C30"));
  palette.setColor(QPalette::ToolTipText,theme=="light" ? QColor("#000000") : QColor("#FCFCFC"));
  palette.setColor(QPalette::PlaceholderText,theme=="light" ? QColor("#000000") : QColor("#FCFCFC"));
  palette.setColor(QPalette::Text,theme=="light" ? QColor("#000000") : QColor("#FCFCFC"));
  palette.setColor(QPalette::Button,theme=="light" ? QColor("#F0F0F0") : QColor("#292C30"));
  palette.setColor(QPalette::ButtonText,theme=="light" ? QColor("#000000") : QColor("#FCFCFC"));
  palette.setColor(QPalette::BrightText,theme=="light" ? QColor("#FFFFFF") : QColor("#FFFFFF"));

  palette.setColor(QPalette::Light,theme=="light" ? QColor("#FFFFFF") : QColor("#151618"));
  palette.setColor(QPalette::Midlight,theme=="light" ? QColor("#E3E3E3") : QColor("#1F2124"));
  palette.setColor(QPalette::Dark,theme=="light" ? QColor("#A0A0A0") : QColor("#525860"));
  palette.setColor(QPalette::Mid,theme=="light" ? QColor("#A0A0A0") : QColor("#373B40"));
  palette.setColor(QPalette::Shadow,theme=="light" ? QColor("#696969") : QColor("#767676"));

  palette.setColor(QPalette::Highlight,theme=="light" ? QColor("#0078D7") : QColor("#3DAEE9"));
  palette.setColor(QPalette::HighlightedText,theme=="light" ? QColor("#FFFFFF") : QColor("#FCFCFC"));

  palette.setColor(QPalette::Link,theme=="light" ? QColor("#0000FF") : QColor("#1D99F3"));
  palette.setColor(QPalette::LinkVisited,theme=="light" ? QColor("#FF00FF") : QColor("#9B59B6"));

  qApp->setPalette(palette);

  // теперь зацепить еще .qss файлы и применить стили
  QFile qssFile(QString(":/styles/%1.qss").arg(theme));
  if (qssFile.open(QIODevice::ReadOnly | QIODevice::Text)==false)
    return;

  qApp->setStyleSheet(qssFile.readAll());
  qssFile.close();

  // поменять иконки у всяческих Actions в соответствии с темой
  exitActn->setIcon(QIcon(QString(":/theme_icons/images/%1/exit").arg(theme)));
  settingsActn->setIcon(QIcon(QString(":/theme_icons/images/%1/settings").arg(theme)));
  archiveActn->setIcon(QIcon(QString(":/theme_icons/images/%1/archive").arg(theme)));
  aboutQtActn->setIcon(QIcon(QString(":/theme_icons/images/%1/qt").arg(theme)));
  aboutActn->setIcon(QIcon(QString(":/theme_icons/images/%1/petcore").arg(theme)));

  appendApptActn->setIcon(QIcon(QString(":/theme_icons/images/%1/appt_append").arg(theme)));
  editApptActn->setIcon(QIcon(QString(":/theme_icons/images/%1/appt_edit").arg(theme)));
  removeApptActn->setIcon(QIcon(QString(":/theme_icons/images/%1/appt_remove").arg(theme)));
  cutApptActn->setIcon(QIcon(QString(":/theme_icons/images/%1/cut").arg(theme)));
  pasteApptActn->setIcon(QIcon(QString(":/theme_icons/images/%1/paste").arg(theme)));
  startApptActn->setIcon(QIcon(QString(":/theme_icons/images/%1/appt_start").arg(theme)));
  finishApptActn->setIcon(QIcon(QString(":/theme_icons/images/%1/appt_finish").arg(theme)));

  appendDataActn->setIcon(QIcon(QString(":/theme_icons/images/%1/append").arg(theme)));
  editDataActn->setIcon(QIcon(QString(":/theme_icons/images/%1/edit").arg(theme)));
  removeDataActn->setIcon(QIcon(QString(":/theme_icons/images/%1/remove").arg(theme)));

  appendRemActn->setIcon(QIcon(QString(":/theme_icons/images/%1/reminder_append").arg(theme)));
  removeRemActn->setIcon(QIcon(QString(":/theme_icons/images/%1/reminder_remove").arg(theme)));

  makeApptActn->setIcon(QIcon(QString(":/theme_icons/images/%1/appt_append").arg(theme)));

  appendTaskActn->setIcon(QIcon(QString(":/theme_icons/images/%1/task_append").arg(theme)));
  editTaskActn->setIcon(QIcon(QString(":/theme_icons/images/%1/task_edit").arg(theme)));
  removeTaskActn->setIcon(QIcon(QString(":/theme_icons/images/%1/task_remove").arg(theme)));
  completeTaskActn->setIcon(QIcon(QString(":/theme_icons/images/%1/task_complete").arg(theme)));

  telegramActn->setIcon(QIcon(QString(":/theme_icons/images/%1/telegram").arg(theme)));

  // поменять иконки табов в m_pagesTabBar в соответствии с темой
  m_pagesTabBar->setTabIcon(TMainWnd::ApptsTab, QIcon(QString(":/theme_icons/images/%1/appts").arg(theme)));
  m_pagesTabBar->setTabIcon(TMainWnd::PetsTab, QIcon(QString(":/theme_icons/images/%1/pets").arg(theme)));
  m_pagesTabBar->setTabIcon(TMainWnd::ClientsTab, QIcon(QString(":/theme_icons/images/%1/clients").arg(theme)));
  m_pagesTabBar->setTabIcon(TMainWnd::DoctorsTab, QIcon(QString(":/theme_icons/images/%1/doctors").arg(theme)));
  m_pagesTabBar->setTabIcon(TMainWnd::LlmTab, QIcon(QString(":/theme_icons/images/%1/llm_vet").arg(theme)));

  // поменять иконки на m_dataToolBar в соответствии с темой
  QStringList actionIcons;
  switch ((TMainWnd::PageTabs)m_pagesTabBar->currentIndex())
    {
    case TMainWnd::PetsTab:
      {
      actionIcons=QStringList{":/theme_icons/images/%1/pet_append",":/theme_icons/images/%1/pet_edit",":/theme_icons/images/%1/pet_remove"};
      break;
      }
    case TMainWnd::ClientsTab:
      {
      actionIcons=QStringList{":/theme_icons/images/%1/client_append",":/theme_icons/images/%1/client_edit",":/theme_icons/images/%1/client_remove"};
      break;
      }
    case TMainWnd::DoctorsTab:
      {
      actionIcons=QStringList{":/theme_icons/images/%1/doctor_append",":/theme_icons/images/%1/doctor_edit",":/theme_icons/images/%1/doctor_remove"};
      break;
      }
    default:
      break;
    }

  QList<QAction*> actions=m_dataToolBar->actions();
  for (quint8 i=0;i<actions.size();i++)
    actions[i]->setIcon(i<actionIcons.size() ? QIcon(actionIcons.at(i).arg(theme)) : QIcon());

  //pageTabChanged(m_pagesTabBar->currentIndex());
  }

void TMainWnd::pageTabChanged(int index)
  {
  // запомнить выделенную строку до смены модельки у таблицы
  QItemSelectionModel *selectionModel=m_lastActiveTab==TMainWnd::ApptsTab ? sheduleTree->selectionModel() : dataTable->selectionModel();
  QModelIndexList indexes=selectionModel==nullptr ? QModelIndexList() : selectionModel->selectedRows();
  QModelIndex current=indexes.size()>0 ? indexes.first() : QModelIndex();

  // в зависимости от того, откуда и куда переключаемся, запомнить в каком поле искать и какое значение искать
  QString keyField;   // ключевое поле, в котором будем искать значение в модели после переключения
  quint32 keyValue=0; // значение поля, которое будем искать
  switch (m_lastActiveTab)
    {
    case TMainWnd::ApptsTab:
      {
      TSheduleModel::Types type=(TSheduleModel::Types)m_sheduleModel->index(current.row(),TSheduleModel::Type,current.parent()).data().toUInt();
      keyField="id"; // при переключении в Pets, Clients, Doctors ключевое поле будет "id"
      switch ((TMainWnd::PageTabs)index)
        {
        case TMainWnd::PetsTab:
          {
          if (type!=TSheduleModel::TimeType)
            break;

          QHash<QString,QVariant> apptHash=m_sheduleModel->index(current.row(),TSheduleModel::DataRecord,current.parent()).data().toHash();
          keyValue=apptHash.value("pet_id").toUInt();
          break;
          }
        case TMainWnd::ClientsTab:
          {
          if (type!=TSheduleModel::TimeType)
            break;

          QHash<QString,QVariant> apptHash=m_sheduleModel->index(current.row(),TSheduleModel::DataRecord,current.parent()).data().toHash();
          keyValue=apptHash.value("client_id").toUInt();
          break;
          }
        case TMainWnd::DoctorsTab:
          {
          if (type==TSheduleModel::DateType)
            break;

          QHash<QString,QVariant> routineHash;
          if (type==TSheduleModel::TimeType)
            routineHash=m_sheduleModel->index(current.parent().row(),TSheduleModel::DataRecord,current.parent().parent()).data().toHash();
          else
            routineHash=m_sheduleModel->index(current.row(),TSheduleModel::DataRecord,current.parent()).data().toHash();

          keyValue=routineHash.value("doctor_id").toUInt();
          break;
          }
        default: // в остальных случаях ничего нигде не ищем
          break;
        }

      break;
      }
    case TMainWnd::PetsTab:
      {
      keyField="id";
      keyValue=m_petsModel->index(current.row(),m_petsModel->fieldIndex("client_id")).data().toInt();
      break;
      }
    case TMainWnd::ClientsTab:
      {
      keyField="client_id";
      keyValue=m_clientsModel->index(current.row(),m_clientsModel->fieldIndex("id")).data().toInt();
      break;
      }
    case TMainWnd::DoctorsTab:
      {
      keyField="doctor_id";
      keyValue=m_doctorsModel->index(current.row(),m_doctorsModel->fieldIndex("id")).data().toInt();
      break;
      }
    default:
      break;
    }

  // сохранить header для отключаемой модели, актуально только если мы были до этого на DataPage, для Shedule header сохраняется в writeSettings()
  QList <TMainWnd::PageTabs> dataTabs={TMainWnd::PetsTab, TMainWnd::ClientsTab, TMainWnd::DoctorsTab};
  if (dataTabs.contains(m_lastActiveTab)==true)
    {
    QString headerStatePath="main_window/data_view/"+pageTabText((TMainWnd::PageTabs)m_lastActiveTab);
    TSettings().setXmlValue(headerStatePath,"",dataTable->horizontalHeader()->saveState());
    }

  // теперь m_lastActiveTab - новый index
  m_lastActiveTab=(TMainWnd::PageTabs)index;

  // показать нужную страницу на pagesStacked, определить модель, скрытые столбцы, названия и иконки Action + восстановить header подключаемой модели
  TSqlTableModel *dataModel=nullptr;
  QStringList hiddenFields;
  QStringList actionIcons;
  QStringList actionTexts;

  switch ((TMainWnd::PageTabs)index)
    {
    case TMainWnd::ApptsTab:
      {
      pagesStacked->setCurrentIndex(TMainWnd::ShedulePage);
      addonStacked->setCurrentIndex(TMainWnd::PeriodAddon);
      filterStacked->setCurrentIndex(TMainWnd::DoctorsFilter);

      controlFrame->show();

      // вызвать построение дерева и уйти из метода, m_dataChanged=true если были изменеия в m_dcotorsModel, m_clientsModel или m_petsModel
      if (m_dataChanged==true)
        {
        buildShedule();
        m_dataChanged=false;
        }

      return;
      }
    case TMainWnd::PetsTab:
      {
      pagesStacked->setCurrentIndex(TMainWnd::DataPage);
      addonStacked->setCurrentIndex(TMainWnd::SearchAddon);
      filterStacked->setCurrentIndex(TMainWnd::PetsFilter);

      dataModel=m_petsModel;

      hiddenFields=QStringList{"id", "client_id", "address", "species_id", "breed_id", "marking_id", "status_id", "forbidden"};
      actionIcons=QStringList{":/theme_icons/images/%1/pet_append",":/theme_icons/images/%1/pet_edit",":/theme_icons/images/%1/pet_remove"};
      actionTexts=QStringList{tr("Append pet"),tr("Edit pet"),tr("Remove pet")};
      break;
      }
    case TMainWnd::ClientsTab:
      {
      pagesStacked->setCurrentIndex(TMainWnd::DataPage);
      addonStacked->setCurrentIndex(TMainWnd::SearchAddon);
      filterStacked->setCurrentIndex(TMainWnd::PetsFilter);
      petsFilterGroup->setEnabled(false);

      dataModel=m_clientsModel;

      hiddenFields=QStringList{"id"};
      actionIcons=QStringList{":/theme_icons/images/%1/client_append",":/theme_icons/images/%1/client_edit",":/theme_icons/images/%1/client_remove"};
      actionTexts=QStringList{tr("Append client"),tr("Edit client"),tr("Remove client")};
      break;
      }
    case TMainWnd::DoctorsTab:
      {
      pagesStacked->setCurrentIndex(TMainWnd::DataPage);
      addonStacked->setCurrentIndex(TMainWnd::SearchAddon);
      filterStacked->setCurrentIndex(TMainWnd::DoctorsFilter);

      dataModel=m_doctorsModel;

      hiddenFields=QStringList{"id", "profile_id", "degree_id", "rank_id", "petage_id", "action_id", "available"};
      actionIcons=QStringList{":/theme_icons/images/%1/doctor_append",":/theme_icons/images/%1/doctor_edit",":/theme_icons/images/%1/doctor_remove"};
      actionTexts=QStringList{tr("Append doctor"),tr("Edit doctor"),tr("Remove doctor")};
      break;
      }
    case TMainWnd::LlmTab:
      {
      pagesStacked->setCurrentIndex(TMainWnd::LlmPage);
      controlFrame->hide();
      questionEdit->setFocus();

      return;
      }
    default:
      break;
    }

  controlFrame->show();

  /* дальше касаемо уже только обработка DataPage */

  QString theme=qApp->property("app_theme").toString();
  // скорректировать конекстное меню в таблице и кнопочки в тулбарев соответствии с текущей вкладкой
  QList<QAction*> actions=m_dataToolBar->actions();
  for (quint8 i=0;i<actions.size();i++)
    {
    actions[i]->setIcon(i<actionIcons.size() ? QIcon(actionIcons.at(i).arg(theme)) : QIcon());
    actions[i]->setText(i<actionTexts.size() ? actionTexts.at(i) : QString());
    actions[i]->setToolTip(i<actionTexts.size() ? actionTexts.at(i) : QString());
    }

  // для клиентов фильтров нету
  petsFilterGroup->setEnabled((TMainWnd::PageTabs)index != TMainWnd::ClientsTab);

  // на всякий проверка, если модель не определена - то уйти
  if (dataModel==nullptr)
    return;

  // обновить модельку (данные могли поменяться), подставить модельку
  dataModel->select();
  dataTable->setModel(dataModel);

  //восстановить хедеры для таблицы
  QString headerStatePath="main_window/data_view/"+pageTabText((TMainWnd::PageTabs)index);
  dataTable->horizontalHeader()->restoreState(TSettings().getXmlValue(headerStatePath,"",QByteArray()).toByteArray());

  // сказать, какие поля скрыть в таблице
  QSqlRecord record=dataModel->record();
  for (quint8 i=0; i<record.count(); i++)
    dataTable->setColumnHidden(i, hiddenFields.contains(record.fieldName(i)));

  // зацепить изменение selection, connect сделать именно с Qt::UniqueConnection, иначе будет много-много срабатываний слота
  connect(dataTable->selectionModel(),&QItemSelectionModel::selectionChanged,this,&TMainWnd::dataSelectionChanged, Qt::UniqueConnection);

  // попробовать найти по ключевому полю и значению ПОСЛЕДНЮЮ запись в новых данных (это важно для посещений - последнее посещение), для остальных last=first
  indexes=dataModel->match(dataModel->index(0, dataModel->fieldIndex(keyField)),Qt::DisplayRole,keyValue,-1,Qt::MatchExactly);
  current=indexes.size()>0 ? indexes.last() : dataModel->index(0, dataModel->fieldIndex(dataModel->headerField()));
  setupDataIndex(current);
  }

QString TMainWnd::pageTabText(PageTabs tab) const
  {
  QString text=QMetaEnum::fromType<TMainWnd::PageTabs>().valueToKey(tab);
  return text.remove("Tab").toLower();
  }

QHash <QString,QVariant> TMainWnd::recordToHash(const QSqlRecord &record)
  {
  QHash <QString,QVariant> dataHash;
  for (quint8 i=0;i<record.count();i++)
    dataHash.insert(record.field(i).name(), record.field(i).value());

  return dataHash;
  }

QSqlRecord TMainWnd::hashToRecord(const QSqlRecord &pattern, const QHash<QString, QVariant> &dataHash)
  {
  QSqlRecord record=pattern;
  for (quint8 i=0;i<record.count();i++)
    record.setValue(i,dataHash.value(record.fieldName(i)));

  return record;
  }

quint32 TMainWnd::createTimeSlot(const QModelIndex &doctorIndex, qint32 timeRow, const QTime &time, bool available)
  {
  m_sheduleModel->insertRow(timeRow,doctorIndex);
  if (timeRow<0)
    timeRow=m_sheduleModel->rowCount(doctorIndex)-1;

  QModelIndex dateIndex=doctorIndex.parent();

  // взять данные из parent - из Доктора, они будут спроецированны в сами тайм-слоты
  QVariantList doctorRecord=m_sheduleModel->record(dateIndex,doctorIndex.row());
  // начальные данные при вставлении любой записи в тайм-слоты (type == TimeType)
  m_sheduleModel->setData(m_sheduleModel->index(timeRow,TSheduleModel::Title,doctorIndex),"Title");
  m_sheduleModel->setData(m_sheduleModel->index(timeRow,TSheduleModel::Date,doctorIndex),doctorRecord.at(TSheduleModel::Date));
  m_sheduleModel->setData(m_sheduleModel->index(timeRow,TSheduleModel::DoctorId,doctorIndex),doctorRecord.at(TSheduleModel::DoctorId));
  m_sheduleModel->setData(m_sheduleModel->index(timeRow,TSheduleModel::Time,doctorIndex),time);
  m_sheduleModel->setData(m_sheduleModel->index(timeRow,TSheduleModel::Type,doctorIndex),TSheduleModel::TimeType);
  m_sheduleModel->setData(m_sheduleModel->index(timeRow,TSheduleModel::DataRecord,doctorIndex),QHash<QString,QVariant>());

  return timeRow;
  }

void TMainWnd::updateTimeSlot(const QModelIndex &doctorIndex, quint32 timeRow, qint32 apptRow)
  {
  // если надо удалить, то присылаем apptRow=-1, и назначаем пустой QSqlRecord()
  QSqlRecord apptRecord=apptRow<0 ? QSqlRecord() : m_apptsModel->record(apptRow);
  m_sheduleModel->setData(m_sheduleModel->index(timeRow,TSheduleModel::DataRecord,doctorIndex),recordToHash(apptRecord));
  }

void TMainWnd::removeTimeSlot(const QModelIndex &doctorIndex, quint32 timeRow)
  {
  m_sheduleModel->removeRow(timeRow, doctorIndex);
  }

void TMainWnd::adjustSheduleActions()
  {
  QModelIndexList indexes=sheduleTree->selectionModel()==nullptr ? QModelIndexList() : sheduleTree->selectionModel()->selectedRows();
  // ткнули "в никуда" - ничего не доступно
  if (indexes.size()==0)
    {
    foreach (QAction *action, sheduleTree->actions())
      action->setEnabled(false);

    return;
    }

  QModelIndex sheduleIndex=indexes.first();
  TSheduleModel::Types type=(TSheduleModel::Types)m_sheduleModel->index(sheduleIndex.row(),TSheduleModel::Type,sheduleIndex.parent()).data().toUInt();
  // скорректировать Title-хедер дерева в соответствии с тем, какой индекс дерева выбран
  m_sheduleModel->setHeaderData(TSheduleModel::Title,Qt::Horizontal,TSheduleModel::titleByType(type),Qt::DisplayRole);

  // если узел не TimeType - все недоступно, кроме telegramActn, отдельно разруливается
  if (type!=TSheduleModel::TimeType)
    {
    foreach (QAction *action, sheduleTree->actions())
      {
      if (action!=telegramActn)
        action->setEnabled(false);
      }

    QHash<QString,QVariant> routineHash;
    if (type==TSheduleModel::DoctorType) // в DataRecord лежат данные из TRoutinesModel
      routineHash=m_sheduleModel->index(sheduleIndex.row(),TSheduleModel::DataRecord,sheduleIndex.parent()).data().toHash();

    telegramActn->setEnabled(routineHash.value("telegram").toString().isEmpty()==false);
    return;
    }

  // если тут, то TimeType, и тут чуть более "ветвисто"
  QDate date=m_sheduleModel->index(sheduleIndex.row(),TSheduleModel::Date,sheduleIndex.parent()).data().toDate();
  QTime time=m_sheduleModel->index(sheduleIndex.row(),TSheduleModel::Time,sheduleIndex.parent()).data().toTime();
  // разница между назначенным и текущим временем, может быть отрицательной
  qint32 delta=QDateTime::currentDateTime().secsTo(QDateTime(date,time));

  // в DataRecord лежат данные из TApptsModel
  QHash<QString,QVariant> apptHash=m_sheduleModel->index(sheduleIndex.row(),TSheduleModel::DataRecord,sheduleIndex.parent()).data().toHash();
  qint32 apptId=apptHash.value("id").toUInt(); // ID записи на прием (0 - если нету)
  TApptsModel::States state=(TApptsModel::States)apptHash.value("state").toUInt(); // состояние записи на прием

  // в DataRecord родителя лежит запись из TRoutinesModel
  QHash<QString,QVariant> routineHash=m_sheduleModel->index(sheduleIndex.parent().row(),TSheduleModel::DataRecord,sheduleIndex.parent().parent()).data().toHash();
  bool available=routineHash.value("available").toBool();  // доступно или нет для записи на прием
  quint16 interval=routineHash.value("interval").toUInt(); // интервал между приемами

  appendApptActn->setEnabled(available==true && apptId==0 && delta>0);
  editApptActn->setEnabled(available==true && apptId>0 && state==TApptsModel::Created && delta>0);
  removeApptActn->setEnabled(apptId>0 && state==TApptsModel::Created && delta>0);

  cutApptActn->setEnabled(apptId>0 && state==TApptsModel::Created && delta>0);
  pasteApptActn->setEnabled(available==true && apptId==0 && delta>0 && m_memApptRecord.isEmpty()==false);

  // стартовать можно, если прошло не более половины интервала или до времени приема не более половины интервала
  startApptActn->setEnabled(available==true && apptId>0 && state==TApptsModel::Created && qAbs(delta)<30*interval);
  finishApptActn->setEnabled(available==true && apptId>0 && state==TApptsModel::Started);

  telegramActn->setEnabled(apptHash.value("telegram").toString().isEmpty()==false);
  }

void TMainWnd::setupSheduleIndex(const QModelIndex &index, quint32 scroll)
  {
  if (index.isValid()==false)
    {
    adjustSheduleActions();
    return;
    }

  // чтобы обновился принудительно
  if (sheduleTree->currentIndex().row()==index.row())
    {
    sheduleTree->setCurrentIndex(QModelIndex());
    sheduleTree->selectionModel()->select(QModelIndex(),QItemSelectionModel::SelectCurrent | QItemSelectionModel::Rows);
    }

  sheduleTree->setCurrentIndex(index);
  sheduleTree->selectionModel()->select(index,QItemSelectionModel::SelectCurrent | QItemSelectionModel::Rows);
  if (scroll>0)
    sheduleTree->verticalScrollBar()->setValue(scroll);

  sheduleTree->setFocus();
  }

QMap <QDate, QList<quint32>> TMainWnd::solveExpanded()
  {
  QMap <QDate, QList<quint32>> expanded;
  quint16 datesSize=m_sheduleModel->rowCount(m_sheduleModel->rootIndex());
  for (quint16 i=0;i<datesSize;i++)
    {
    QModelIndex dateIndex=m_sheduleModel->index(i,TSheduleModel::Title,m_sheduleModel->rootIndex());
    if (sheduleTree->isExpanded(dateIndex)==false)
      continue;

    QDate date=m_sheduleModel->index(i,TSheduleModel::Date,m_sheduleModel->rootIndex()).data().toDate();
    expanded.insert(date, QList<quint32>());

    QList <quint32> doctors;
    quint16 doctorSize=m_sheduleModel->rowCount(dateIndex);
    for (quint16 j=0;j<doctorSize;j++)
      {
      QModelIndex doctorIndex=m_sheduleModel->index(j,TSheduleModel::Title,dateIndex);
      if (sheduleTree->isExpanded(doctorIndex)==true)
        {
        // в DataRecord лежат данные из TRoutinesModel
        QHash<QString,QVariant> routineHash=m_sheduleModel->index(j,TSheduleModel::DataRecord,dateIndex).data().toHash();
        quint32 doctorId=routineHash.value("doctor_id").toUInt();
        doctors << doctorId;
        }
      }

    expanded[date]=doctors;
    }

  return expanded;
  }

void TMainWnd::buildDateBranch(const QModelIndex &dateIndex)
  {
//qDebug()<<"build date";
  // тут реальное построение ветки Даты
  // посчитать порядковый номер дня, исходя из "понедельник нечетной недели = 0, воскресенье четной недели = 13"
  QDate date=m_sheduleModel->index(dateIndex.row(),TSheduleModel::Date,dateIndex.parent()).data().toDate();
  quint8 day=((date.weekNumber()+1)%2)*7+(date.dayOfWeek()-1);

  // сначала отфильтровать оригинал по дню + фильтр докторов, потом применить фильтрующую модель с type=Available,
  if (doctorsFilterGroup->isChecked()==true)
    {
    // применить фильтр из таблицы докторов
    QString doctorFilter=m_doctorsModel->filter();
    if (doctorFilter.isEmpty()==false)
      m_routinesModel->setFilter(QString("routines.doctor_id in (select doctors.id from doctors where %1)").arg(doctorFilter));
    }
  else
    m_routinesModel->setFilter("");

  qApp->setOverrideCursor(Qt::WaitCursor);

  TRoutinesFilterModel existRoutineModel(TRoutinesFilterModel::Available);
  existRoutineModel.setSourceModel(m_routinesModel);
  existRoutineModel.setFilterData(day);

  m_sheduleModel->clear(dateIndex);
  for (quint16 i=0;i<existRoutineModel.rowCount();i++)
    {
    // это индекс расписания доктора, его хранить для добычи данных про рабочий день доктора (но можно и для добычи данных про доктора)
    QModelIndex routineIndex=existRoutineModel.mapToSource(existRoutineModel.index(i,m_routinesModel->fieldIndex("id")));
    QSqlRecord routineRecord=m_routinesModel->record(routineIndex.row());

    m_sheduleModel->insertRow(-1,dateIndex);
    quint16 row=m_sheduleModel->rowCount(dateIndex)-1;

    m_sheduleModel->setData(m_sheduleModel->index(row,TSheduleModel::Title,dateIndex),"Title");
    m_sheduleModel->setData(m_sheduleModel->index(row,TSheduleModel::Date,dateIndex),date);
    m_sheduleModel->setData(m_sheduleModel->index(row,TSheduleModel::DoctorId,dateIndex),routineRecord.value("doctor_id").toUInt());
    m_sheduleModel->setData(m_sheduleModel->index(row,TSheduleModel::Time,dateIndex),QTime(23,59,59));
    m_sheduleModel->setData(m_sheduleModel->index(row,TSheduleModel::Type,dateIndex),TSheduleModel::DoctorType);
    m_sheduleModel->setData(m_sheduleModel->index(row,TSheduleModel::DataRecord, dateIndex),recordToHash(routineRecord));
    }

  qApp->restoreOverrideCursor();
  if (sheduleTree->isExpanded(dateIndex)==true || m_sheduleModel->rowCount(dateIndex)==0)
    return;

  disconnect(sheduleTree,&QTreeView::expanded,this,&TMainWnd::sheduleTreeExpanded);
  sheduleTree->expand(dateIndex);
  connect(sheduleTree,&QTreeView::expanded,this,&TMainWnd::sheduleTreeExpanded);
  }

void TMainWnd::buildDoctorBranch(const QModelIndex &doctorIndex)
  {
//qDebug()<<"build doctor";
  // тут реальное построение ветки Доктора
  QDate date=m_sheduleModel->index(doctorIndex.row(),TSheduleModel::Date,doctorIndex.parent()).data().toDate();

  // в DataRecord лежат данные из TRoutinesModel
  QHash<QString,QVariant> routineHash=m_sheduleModel->index(doctorIndex.row(),TSheduleModel::DataRecord,doctorIndex.parent()).data().toHash();

  quint32 doctorId=routineHash.value("doctor_id").toUInt();
  bool available=routineHash.value("available").toBool();
  quint16 interval=routineHash.value("interval").toUInt();

  QDateTime fromAm=QDateTime(date,routineHash.value("from_am").toTime());
  QDateTime toAm=QDateTime(date,routineHash.value("to_am").toTime());
  QDateTime fromPm=QDateTime(date,routineHash.value("from_pm").toTime());
  QDateTime toPm=QDateTime(date,routineHash.value("to_pm").toTime());

  m_apptsModel->setFilter(QString("appts.doctor_id=%1").arg(doctorId));
  // дальше, чтобы не делать запросы к БД по времени, буду фильтровать уже находящуюся в памяти модель
  TApptsFilterModel apptsFilterModel;
  apptsFilterModel.setSourceModel(m_apptsModel);

  qApp->setOverrideCursor(Qt::WaitCursor);

  // slotType - какие тайм-слоты строить (Все, Актуальные, Прошедшие)
  TMainWnd::SlotTypes slotType=(TMainWnd::SlotTypes)timeslotCombo->currentData().toUInt();
  // slotTime - время начала приема, slotTime.addSecs(60*interval) - время окончания приема
  QDateTime slotTime=fromAm;
  // определяем тайм-слоты, и смотрим назначенные записи, время окончания записи должно укладываться в рабочее время, и не должно быть перехода через полночь
  m_sheduleModel->clear(doctorIndex);
  while (slotTime.addSecs(60*(interval-1))<=toPm) // -1 мин. тут - чтобы зацепить 00:00, если работает до 23:59
    {
    // если нужны только актуальные таймслоты, но slotTime "старый" - то вычисляем следующий slotTime и пропускаем
    if (slotType==TMainWnd::ActualTimeSlots && slotTime.addSecs(60*interval)<QDateTime::currentDateTime())
      {
      slotTime=slotTime.addSecs(60*interval);
      continue;
      }

    // если нужны только прошедшие тайм-слоты, но slotTime уже "свежий" - то вообще уходим, все остальные slotTime будут еще позже
    if (slotType==TMainWnd::ExpiredTimeSlots && slotTime.addSecs(60*interval)>=QDateTime::currentDateTime())
      break;

    quint32 timeRow;
    quint32 apptRow;
    // Отследить перерыв на обед (окончание = toAm - начало = fromPm)
    if (slotTime.addSecs(60*interval)>toAm && slotTime<fromPm)
      {
      apptsFilterModel.setFilterData(slotTime,fromPm,{});
      for (quint8 i=0;i<apptsFilterModel.rowCount();i++)
        {
        QDateTime apptTime=QDateTime(apptsFilterModel.index(i,m_apptsModel->fieldIndex("appt_date")).data().toDate(),
                                     apptsFilterModel.index(i,m_apptsModel->fieldIndex("appt_time")).data().toTime());

        timeRow=createTimeSlot(doctorIndex,-1,apptTime.time(),false);

        // допинывать запись в sheduleModel данными из apptModel
        apptRow=apptsFilterModel.mapToSource(apptsFilterModel.index(i,0)).row();
        updateTimeSlot(doctorIndex,timeRow,apptRow);
        }

      // следующий тайм-слот уже с начала приема "после обеда"
      slotTime=fromPm;
      continue;
      }

    // если не перерыв и slotTime подпадает под условие slottimeCombo, то вставляем запись тайм-слота
    timeRow=createTimeSlot(doctorIndex,-1,slotTime.time(),available);

    // если начало дня - то надо посмотреть, нет ли записей более ранних, если конец дня - нет ли более поздних
    QDateTime fromTime=(slotTime==fromAm) ? QDateTime(date,QTime(0,0,0)) : slotTime; // это первый тайм-слот в расписании
    QDateTime toTime=(slotTime.addSecs(2*60*interval)>toPm) ? QDateTime(date,QTime(23,59,59)) : slotTime.addSecs(60*interval); // это последний тайм-слот в расписании

    apptsFilterModel.setFilterData(fromTime,toTime,{});

    // пройти по всем отфильрованным записям, если есть строгое попадание - то ставим в тайм-слот данные,
    // если нету - вставляем записи в расписание, они будут "протухшие" по определению,
    // учитываем время, если apptTime > slotTime - вставляем после текущей, если apptTime < slotTime - перед текущей
    for (quint8 i=0;i<apptsFilterModel.rowCount();i++)
      {
      QDateTime apptTime=QDateTime(apptsFilterModel.index(i,m_apptsModel->fieldIndex("appt_date")).data().toDate(),
                                   apptsFilterModel.index(i,m_apptsModel->fieldIndex("appt_time")).data().toTime());

      if (apptTime<slotTime) // прием раньше, чем тайм-слот
        timeRow=createTimeSlot(doctorIndex,timeRow,apptTime.time(),false);

      if (apptTime>slotTime) // прием позднее, чем тайм-слот
        timeRow=createTimeSlot(doctorIndex,-1,apptTime.time(),false);

      // допинывать запись в sheduleModel данными из apptModel
      apptRow=apptsFilterModel.mapToSource(apptsFilterModel.index(i,0)).row();
      updateTimeSlot(doctorIndex,timeRow,apptRow);
      }

    // следующий тацм-слот в расписании
    slotTime=slotTime.addSecs(60*interval);
    }

  qApp->restoreOverrideCursor();
  if (sheduleTree->isExpanded(doctorIndex)==true || m_sheduleModel->rowCount(doctorIndex)==0)
    return;

  disconnect(sheduleTree,&QTreeView::expanded,this,&TMainWnd::sheduleTreeExpanded);
  sheduleTree->expand(doctorIndex);
  connect(sheduleTree,&QTreeView::expanded,this,&TMainWnd::sheduleTreeExpanded);
  }

void TMainWnd::buildShedule()
  {
  // запомнить, где находимся в дереве и запомнить все expanded узлы дерева
  QModelIndexList indexes=sheduleTree->selectionModel()->selectedRows();
  QModelIndex index=indexes.size()>0 ? m_sheduleModel->index(indexes.first().row(),TSheduleModel::Title,indexes.first().parent()) : QModelIndex();
  QVariantList path=m_sheduleModel->fullPath(index);

  // дерево будет перестоено и индексы буду иные, надо запомнить, кто expanded
  QMap <QDate, QList<quint32>> expanded=solveExpanded();
  QDate fromDate=QDate::currentDate();
  QDate toDate=fromDate.addDays(6);
  if (startDateEdit->date()>QDate(2000,1,1) && endDateEdit->date()>QDate(2000,1,1))
    {
    fromDate=startDateEdit->date();
    toDate=endDateEdit->date();
    }

  // запомнить положение скролла
  quint32 scroll=sheduleTree->verticalScrollBar()->value();
  // по сути построить дерево в прежнем (по мере возможности) виде
  m_sheduleModel->setPeriod(fromDate,toDate);
  expandSheduleNodes(expanded);

  // попробовать найти выделенный перед перестроением дерева индекс
  index=m_sheduleModel->findMatch(path);
  setupSheduleIndex(index,scroll);
  }

void TMainWnd::expandSheduleNodes(const QMap<QDate, QList<quint32> > &expanded)
  {
  if (m_dataChanged==true)
    {
    m_apptsModel->select();
    m_dataChanged=false;
    }

  foreach (const QDate &dateToExpand, expanded.keys())
    {
    QModelIndex indexToExpand=m_sheduleModel->findMatch({dateToExpand});
    if (indexToExpand.isValid()==true)
      buildDateBranch(indexToExpand);

    QList<quint32> doctors=expanded.value(dateToExpand);
    foreach (quint32 doctorToExpand, doctors)
      {
      indexToExpand=m_sheduleModel->findMatch({dateToExpand, doctorToExpand});
      if (indexToExpand.isValid()==true)
        buildDoctorBranch(indexToExpand);
      }
    }
  }

void TMainWnd::showSheduleAppts(QMap<QDate, QList<quint32>> expanded)
  {
  // установить период дат, чтобы точно охватывал крайние даты приемов
  QPair<QDate,QDate> dates=m_sheduleModel->period();
  QDate fromDate=qMin(expanded.keys().first(), dates.first) ;
  QDate toDate=qMax(expanded.keys().last(), dates.second);

  startDateEdit->setDate(fromDate);
  endDateEdit->setDate(toDate);

  m_sheduleModel->setPeriod(fromDate, toDate);
  expandSheduleNodes(expanded);

  // если это возможно, выделить первую дату и доктора в этой дате
  if (expanded.size()>0 && expanded.first().size()>0)
    {
    QModelIndex index=m_sheduleModel->findMatch({expanded.firstKey(),expanded.first().first()});
    setupSheduleIndex(index,0);
    }

  m_pagesTabBar->setCurrentIndex(TMainWnd::ApptsTab);
  }

void TMainWnd::renewTimeSlots()
  {
  TMainWnd::PageTabs pageTab=(TMainWnd::PageTabs)m_pagesTabBar->currentIndex();
  TMainWnd::SlotTypes slotType=(TMainWnd::SlotTypes)timeslotCombo->currentData().toUInt();

  bool dataChanged=false;
  // сначала надо убрать или добавить в дерево тайм-слоты, только если находимся на вкладке Расписания и хотим видеть не все тайм-слоты
  if (pageTab==TMainWnd::ApptsTab && slotType!=TMainWnd::AllTimeSlots)
    {
    QMap<QDate, QList<quint32>> expanded=solveExpanded();
    foreach (const QDate &date, expanded.keys())
      {
      QList<quint32> doctors=expanded.value(date);
      foreach (quint32 doctor, doctors)
        {
        QModelIndex doctorIndex=m_sheduleModel->findMatch({date, doctor});
        // в DataRecord лежат данные из TRoutinesModel
        QHash<QString,QVariant> routineHash=m_sheduleModel->index(doctorIndex.row(),TSheduleModel::DataRecord,doctorIndex.parent()).data().toHash();
        quint16 interval=routineHash.value("interval").toUInt();

        for (quint32 row=0;row<m_sheduleModel->rowCount(doctorIndex);row++)
          {
          QTime slotTime=m_sheduleModel->index(row,TSheduleModel::Time,doctorIndex).data().toTime();
          // если отображаем только Актуальные и тайм-слот стал Прошедшим
          if (slotType==TMainWnd::ActualTimeSlots && QDateTime(date,slotTime.addSecs(60*interval))<QDateTime::currentDateTime())
            {
            removeTimeSlot(doctorIndex,row);
            dataChanged=true;
            break; // за одно срабатывание таймера при нахождении а Расписании только один тайм-слот может стать Прошедшим
            }
          // если отображаем только Прошедшие и тайм-слот стал Прошедшим
          if (slotType==TMainWnd::ExpiredTimeSlots && QDateTime(date,slotTime.addSecs(60*interval))>=QDateTime::currentDateTime())
            {
            createTimeSlot(doctorIndex,-1,slotTime,false);
            dataChanged=true;
            break; // за одно срабатывание таймера при нахождении а Расписании только один тайм-слот может стать Прошедшим
            }
          }
        }
      }
    }

  // а затем надо скорректировать записи на прием
  TApptsFilterModel apptsFilterModel;
  apptsFilterModel.setSourceModel(m_apptsModel);

  // отфильтруем - мне тут не нужны Завершенные приемы, которых может быть много
  apptsFilterModel.setFilterData(QDateTime(),QDateTime(),{TApptsModel::Created,TApptsModel::Started});
  for (qint32 i=apptsFilterModel.rowCount()-1;i>-1;i--)
    {
    // сразу через оригинальную модель работать
    QModelIndex apptIndex=apptsFilterModel.mapToSource(apptsFilterModel.index(i,0));
    QSqlRecord apptRecord=m_apptsModel->record(apptIndex.row());

    quint32 doctorId=apptRecord.value("doctor_id").toUInt();
    QDate date=apptRecord.value("appt_date").toDate();
    QTime time=apptRecord.value("appt_time").toTime();
    qint32 delta=QDateTime(date,time).secsTo(QDateTime::currentDateTime());
    // interval также тянется в TApptsModel, можно взять
    quint16 interval=apptRecord.value("interval").toUInt();

    QModelIndex doctorIndex=m_sheduleModel->findMatch({date,doctorId});
    QModelIndex timeIndex=m_sheduleModel->findMatch({date,doctorId,time});

    // виден ли timeIndex в настоящее время на экране
    bool visible=(pageTab==TMainWnd::ApptsTab && sheduleTree->isExpanded(doctorIndex)==true && timeIndex.isValid()==true);
    TApptsModel::States state=(TApptsModel::States)apptRecord.value("state").toUInt();;

    // если время приема прошло уже "половину интервала" назад
    if (state==TApptsModel::Created && delta>=30*interval)
      {
      m_apptsModel->removeRow(apptIndex.row());
      m_apptsModel->submit();

      // обновлять данные в индексе, надо только, если он виден сейчас на экране, в противном случае он перерисуется при раскрытии/построении
      if (visible==true)
        {
        updateTimeSlot(timeIndex.parent(),timeIndex.row(),apptIndex.row());
        dataChanged=true;
        }

      continue;
      }

    // если прием уже стартовал уже "интервал" назад
    if (state==TApptsModel::Started && delta>=60*interval)
      {
      m_apptsModel->setData(m_apptsModel->index(apptIndex.row(),m_apptsModel->fieldIndex("state")),(quint8)TApptsModel::Finished);
      m_apptsModel->submit();

      // обновлять данные в индексе, надо только, если он виден сейчас на экране, в противном случае он перерисуется при раскрытии/построениие
      if (visible==true)
        {
        updateTimeSlot(timeIndex.parent(),timeIndex.row(),apptIndex.row());
        dataChanged=true;
        }

      continue;
      }
    }

  // если находимся НЕ на вкладке Расписания или данные были изменены (само изменени уже спровоцировало перерисовку) - то можно уходить
  if (pageTab==TMainWnd::ApptsTab && dataChanged==true)
    {
    sheduleTree->hide();
    sheduleTree->show();
    }

  // подрулить время следующего вызова, таймер должен сработать через 5 секунд после начала следующей минуты
  QDateTime dateTime=QDateTime::currentDateTime();
  dateTime=dateTime.addSecs(60-dateTime.time().second()+5);
  quint16 msecs=QDateTime::currentDateTime().msecsTo(dateTime);

  QTimer::singleShot(msecs,Qt::PreciseTimer,this,&TMainWnd::renewTimeSlots);
  }

QModelIndex TMainWnd::lastApptIndex()
  {
  qint32 apptId=m_dataModule->lastAutoInc("appts");
  QModelIndexList indexes=m_apptsModel->match(m_apptsModel->index(0,m_apptsModel->fieldIndex("id")),Qt::DisplayRole,apptId,1,Qt::MatchExactly);
  if (indexes.size()==0)
    return QModelIndex();

  return indexes.first();
  }

void TMainWnd::appendAppt()
  {
  QModelIndexList indexes=sheduleTree->selectionModel()->selectedRows();
  if (indexes.size()==0)
    return;

  QModelIndex timeIndex=indexes.first();
  QModelIndex doctorIndex=timeIndex.parent();

  // в DataRecord родителя лежит запись из TRoutinesModel
  QHash<QString,QVariant> routineHash=m_sheduleModel->index(doctorIndex.row(),TSheduleModel::DataRecord,doctorIndex.parent()).data().toHash();
  QSqlRecord apptRecord=m_dataModule->tableRecord("appts");

  // сразу устанавливаем данные согласно записи на которой находимся, это id доктора, дата и время
  apptRecord.setValue("doctor_id",routineHash.value("doctor_id").toUInt());
  apptRecord.setValue("appt_date",m_sheduleModel->index(timeIndex.row(),TSheduleModel::Date,doctorIndex).data());
  apptRecord.setValue("appt_time",m_sheduleModel->index(timeIndex.row(),TSheduleModel::Time,doctorIndex).data());

  TApptDlg apptDlg(TMainWnd::ApptsTab,apptRecord,this);
  connect(&apptDlg,&TApptDlg::updateDataNeeded,this,&TMainWnd::updateData);
  if (apptDlg.exec()!=QDialog::Accepted)
    return;

  apptRecord=apptDlg.record();
  m_apptsModel->insertRecord(-1, apptRecord);

  QModelIndex apptIndex=lastApptIndex();
  if (apptIndex.isValid()==false)
    return;

  // запомнить положение скролла
  quint32 scroll=sheduleTree->verticalScrollBar()->value();
  updateTimeSlot(doctorIndex,timeIndex.row(),apptIndex.row());

  timeIndex=m_sheduleModel->findMatch({apptRecord.value("appt_date").toDate(),apptRecord.value("doctor_id").toUInt(),apptRecord.value("appt_time").toTime()});
  setupSheduleIndex(timeIndex, scroll);
  }

void TMainWnd::makeAppt()
  {
  TSqlTableModel *dataModel=qobject_cast<TSqlTableModel*>(dataTable->model());
  if (dataModel==nullptr)
    return;

  QModelIndexList indexes=dataTable->selectionModel()->selectedRows();
  if (indexes.size()==0)
    return;

  QModelIndex index=indexes.first();
  QSqlRecord apptRecord=m_dataModule->tableRecord("appts");

  TMainWnd::PageTabs pageTab=(TMainWnd::PageTabs)m_pagesTabBar->currentIndex();
  switch (pageTab)
    {
    case TMainWnd::PetsTab:
      {
      QSqlRecord petRecord=m_petsModel->record(index.row());
      apptRecord.setValue("pet_id",petRecord.value("id"));
      apptRecord.setValue("client_id",petRecord.value("client_id"));
      break;
      }
    case TMainWnd::ClientsTab:
      {
      QSqlRecord clientRecord=m_clientsModel->record(index.row());
      apptRecord.setValue("client_id",clientRecord.value("id"));
      break;
      }
    case TMainWnd::DoctorsTab:
      {
      QSqlRecord doctorRecord=m_doctorsModel->record(index.row());
      apptRecord.setValue("doctor_id",doctorRecord.value("id"));
      break;
      }
    default:
      return;
    }

  TApptDlg apptDlg(pageTab,apptRecord,this);
  connect(&apptDlg,&TApptDlg::updateDataNeeded,this,&TMainWnd::updateData);
  if (apptDlg.exec()!=QDialog::Accepted)
    {
    setupDataIndex(index);
    return;
    }

  apptRecord=apptDlg.record();
  m_apptsModel->insertRecord(-1, apptRecord);
  QModelIndex apptIndex=lastApptIndex();
  if (apptIndex.isValid()==false)
    return;

  QString message=tr("A new appointment has been successfully created.\nDisplay appointment on 'Shedule' tab?");
  if (QMessageBox::question(this,tr("Confirmation"), message, QMessageBox::Yes | QMessageBox::No)==QMessageBox::No)
    {
    setupDataIndex(index);
    return;
    }

  QModelIndex doctorIndex=m_sheduleModel->findMatch({apptRecord.value("appt_date"), apptRecord.value("doctor_id")});
  QModelIndex timeIndex;
  if (doctorIndex.isValid()==false || sheduleTree->isExpanded(doctorIndex)==false)
    {
    QMap <QDate, QList<quint32>> expanded;
    expanded.insert(apptRecord.value("appt_date").toDate(), {apptRecord.value("doctor_id").toUInt()});
    expandSheduleNodes(expanded);

    timeIndex=m_sheduleModel->findMatch({apptRecord.value("appt_date").toDate(),apptRecord.value("doctor_id").toUInt(),apptRecord.value("appt_time").toTime()});
    }
  else
    {
    timeIndex=m_sheduleModel->findMatch({apptRecord.value("appt_date").toDate(),apptRecord.value("doctor_id").toUInt(),apptRecord.value("appt_time").toTime()});
    updateTimeSlot(doctorIndex,timeIndex.row(),apptIndex.row());
    }

  setupSheduleIndex(timeIndex,0);
  m_pagesTabBar->setCurrentIndex(TMainWnd::ApptsTab);
  }

void TMainWnd::editAppt()
  {
  QModelIndexList indexes=sheduleTree->selectionModel()->selectedRows();
  if (indexes.size()==0)
    return;

  QModelIndex timeIndex=indexes.first();
  QModelIndex doctorIndex=timeIndex.parent();

  // в DataRecord лежат данные из TApptsModel
  QHash<QString,QVariant> apptHash=m_sheduleModel->index(timeIndex.row(),TSheduleModel::DataRecord,doctorIndex).data().toHash();
  indexes=m_apptsModel->match(m_apptsModel->index(0,m_apptsModel->fieldIndex("id")),Qt::DisplayRole,apptHash.value("id"),1,Qt::MatchExactly);
  if (indexes.size()==0)
    return;

  // сам apptRecord взять из шаблонной записи, а потом проставить значения из attpHash
  QSqlRecord apptRecord=hashToRecord(m_dataModule->tableRecord("appts"),apptHash);

  TApptDlg apptDlg((TMainWnd::PageTabs)m_pagesTabBar->currentIndex(),apptRecord,this);
  connect(&apptDlg,&TApptDlg::updateDataNeeded,this,&TMainWnd::updateData);
  if (apptDlg.exec()!=QDialog::Accepted)
    return;

  QModelIndex apptIndex=indexes.first();
  apptRecord=apptDlg.record();
  m_apptsModel->setRecord(apptIndex.row(),apptRecord);
  m_apptsModel->select();

  updateTimeSlot(doctorIndex,timeIndex.row(),apptIndex.row());
  setupSheduleIndex(timeIndex,0);
  }

void TMainWnd::removeAppt()
  {
  QModelIndexList indexes=sheduleTree->selectionModel()->selectedRows();
  if (indexes.size()==0)
    return;

  if (QMessageBox::question(this,tr("Confirmation"),tr("Do you really want to remove selected record?"),QMessageBox::Yes | QMessageBox::No)==QMessageBox::No)
    return;

  QModelIndex timeIndex=indexes.first();
  QModelIndex doctorIndex=timeIndex.parent();

  // в DataRecord лежат данные из TApptsModel
  QHash<QString,QVariant> apptHash=m_sheduleModel->index(timeIndex.row(),TSheduleModel::DataRecord,doctorIndex).data().toHash();
  indexes=m_apptsModel->match(m_apptsModel->index(0,m_apptsModel->fieldIndex("id")),Qt::DisplayRole,apptHash.value("id"),1,Qt::MatchExactly);
  if (indexes.size()==0)
    return;

  QModelIndex apptIndex=indexes.first();
  m_apptsModel->removeRow(apptIndex.row());
  m_apptsModel->select();

  updateTimeSlot(doctorIndex,timeIndex.row(),-1);
  adjustSheduleActions();

  // Заблокировать возможность удаления на 20 сек
  bool removeEnabled=removeApptActn->isEnabled();
  removeApptActn->setEnabled(false);
  QTimer::singleShot(20000,Qt::PreciseTimer, this,[this,removeEnabled]() {
    removeApptActn->setEnabled(removeEnabled);
    });
  }

void TMainWnd::cutAppt()
  {
  QModelIndexList indexes=sheduleTree->selectionModel()->selectedRows();
  if (indexes.size()==0)
    return;

  QModelIndex timeIndex=indexes.first();
  QModelIndex doctorIndex=timeIndex.parent();

  // в DataRecord лежат данные из TApptsModel
  QHash<QString,QVariant> apptHash=m_sheduleModel->index(timeIndex.row(),TSheduleModel::DataRecord,doctorIndex).data().toHash();
  indexes=m_apptsModel->match(m_apptsModel->index(0,m_apptsModel->fieldIndex("id")),Qt::DisplayRole,apptHash.value("id"),1,Qt::MatchExactly);
  if (indexes.size()==0)
    return;

  QModelIndex apptIndex=indexes.first();
  // запомнить нужные данные в m_memApptRecord, сам m_memApptRecord - шаблонная запись, т.е. именно запись из таблицы "appts"
  m_memApptRecord=hashToRecord(m_dataModule->tableRecord("appts"),apptHash);

  // удалить запись из m_apptModel
  m_apptsModel->removeRow(apptIndex.row());
  m_apptsModel->select();

  updateTimeSlot(doctorIndex,timeIndex.row(),-1);
  adjustSheduleActions();
  }

void TMainWnd::pasteAppt()
  {
  QModelIndexList indexes=sheduleTree->selectionModel()->selectedRows();
  if (indexes.size()==0 || m_memApptRecord.isEmpty()==true)
    return;

  QModelIndex timeIndex=indexes.first();
  QModelIndex doctorIndex=timeIndex.parent();

  // допинать m_apptRecord текущими данными (куда ткнули "вставить" в sheduleTree)
  m_memApptRecord.setValue("doctor_id", m_sheduleModel->index(timeIndex.row(),TSheduleModel::DoctorId,doctorIndex).data().toUInt());
  m_memApptRecord.setValue("appt_date", m_sheduleModel->index(timeIndex.row(),TSheduleModel::Date,doctorIndex).data().toDate());
  m_memApptRecord.setValue("appt_time", m_sheduleModel->index(timeIndex.row(),TSheduleModel::Time,doctorIndex).data().toTime());

  // получившуюся m_apptRecord (теперь уже полностью заполненную) пихнуть в m_apptModel
  m_apptsModel->insertRecord(-1,m_memApptRecord);
  QModelIndex apptIndex=lastApptIndex();
  if (apptIndex.isValid()==false)
    return;

  m_memApptRecord.clear();
  updateTimeSlot(doctorIndex,timeIndex.row(),apptIndex.row());
  adjustSheduleActions();
  }

void TMainWnd::startAppt()
  {
  QModelIndexList indexes=sheduleTree->selectionModel()->selectedRows();
  if (indexes.size()==0)
    return;

  QModelIndex timeIndex=indexes.first();
  QModelIndex doctorIndex=timeIndex.parent();

  // в DataRecord лежат данные из TApptsModel
  QHash<QString,QVariant> apptHash=m_sheduleModel->index(timeIndex.row(),TSheduleModel::DataRecord,doctorIndex).data().toHash();
  indexes=m_apptsModel->match(m_apptsModel->index(0,m_apptsModel->fieldIndex("id")),Qt::DisplayRole,apptHash.value("id"),1,Qt::MatchExactly);
  if (indexes.size()==0)
    return;

  // нельзя начать не назначенный прием
  if ((TApptsModel::States)apptHash.value("state").toUInt()!=TApptsModel::Created)
    return;

  QModelIndex apptIndex=indexes.first();
  m_apptsModel->setData(m_apptsModel->index(apptIndex.row(),m_apptsModel->fieldIndex("state")),TApptsModel::Started);
  m_apptsModel->submit();

  updateTimeSlot(doctorIndex,timeIndex.row(),apptIndex.row());
  adjustSheduleActions();
  }

void TMainWnd::finishAppt()
  {
  QModelIndexList indexes=sheduleTree->selectionModel()->selectedRows();
  if (indexes.size()==0)
    return;

  QModelIndex timeIndex=indexes.first();
  QModelIndex doctorIndex=timeIndex.parent();

  // в DataRecord лежат данные из TApptsModel
  QHash<QString,QVariant> apptHash=m_sheduleModel->index(timeIndex.row(),TSheduleModel::DataRecord,doctorIndex).data().toHash();
  indexes=m_apptsModel->match(m_apptsModel->index(0,m_apptsModel->fieldIndex("id")),Qt::DisplayRole,apptHash.value("id"),1,Qt::MatchExactly);
  if (indexes.size()==0)
    return;

  // нельзя окончить не начатый прием
  if ((TApptsModel::States)apptHash.value("state").toUInt()!=TApptsModel::Started)
    return;

  QModelIndex apptIndex=indexes.first();
  m_apptsModel->setData(m_apptsModel->index(apptIndex.row(),m_apptsModel->fieldIndex("state")),TApptsModel::Finished);
  m_apptsModel->submit();

  updateTimeSlot(doctorIndex,timeIndex.row(),apptIndex.row());
  adjustSheduleActions();
  }

void TMainWnd::sheduleTreeExpanded(const QModelIndex &index)
  {
  if (index.isValid()==false)
    return;

  TSheduleModel::Types type=(TSheduleModel::Types)m_sheduleModel->index(index.row(),TSheduleModel::Type,index.parent()).data().toUInt();
  switch (type)
    {
    case TSheduleModel::DateType: // ткнули в Дату
      {
      buildDateBranch(index);
      break;
      }
    case TSheduleModel::DoctorType: // ткнули в Доктора
      {
      buildDoctorBranch(index);
      break;
      }
    default:
      break;
    }
  }

void TMainWnd::sheduleTreeActivated(const QModelIndex &index)
  {
  if (index.isValid()==false)
    return;

  if (sheduleTree->isExpanded(index)==true)
    {
    sheduleTree->collapse(index);
    return;
    }

  TSheduleModel::Types type=(TSheduleModel::Types)m_sheduleModel->index(index.row(),TSheduleModel::Type,index.parent()).data().toUInt();
  switch (type)
    {
    case TSheduleModel::DateType: // ткнули в Дату
      {
      buildDateBranch(index);
      break;
      }
    case TSheduleModel::DoctorType: // ткнули в Доктора
      {
      buildDoctorBranch(index);
      break;
      }
    case TSheduleModel::TimeType: // ткнули в Запись о приеме
      {
      // в DataRecord лежат данные из TApptsModel
      QHash<QString,QVariant> apptHash=m_sheduleModel->index(index.row(),TSheduleModel::DataRecord,index.parent()).data().toHash();
      quint32 apptId=apptHash.value("id").toUInt();
      // если ApptId = 0, то это новая запись, если > 0 - то редактируем
      if (apptId==0 && appendApptActn->isEnabled()==true)
        {
        appendAppt();
        break;
        }

      if (apptId>0 && editApptActn->isEnabled()==true)
        editAppt();

      break;
      }
    }
  }

void TMainWnd::sheduleSelectionChanged(const QItemSelection &selected, const QItemSelection &deselected)
  {
  Q_UNUSED(selected)
  Q_UNUSED(deselected)

  adjustSheduleActions();
  }

void TMainWnd::adjustDataActions()
  {
  if ((TMainWnd::PageTabs)m_pagesTabBar->currentIndex()==TMainWnd::LlmTab)
    return;

  QModelIndexList indexes=dataTable->selectionModel()==nullptr ? QModelIndexList() : dataTable->selectionModel()->selectedRows();
  QModelIndex index=indexes.size()>0 ? indexes.first() : QModelIndex();

  appendDataActn->setEnabled(true);
  editDataActn->setEnabled(index.isValid());
  removeDataActn->setEnabled(index.isValid());

  TSqlTableModel *model=qobject_cast<TSqlTableModel*>(dataTable->model());
  telegramActn->setEnabled(index.isValid() && model->index(index.row(),model->fieldIndex("telegram")).data().toString().isEmpty()==false);

  TMainWnd::PageTabs pageTab=(TMainWnd::PageTabs)m_pagesTabBar->currentIndex();
  // подкорректировать, что видно/не видно в контекстном меню
  appendRemActn->setVisible(pageTab!=TMainWnd::DoctorsTab);
  appendRemActn->setEnabled(index.isValid());
  makeApptActn->setEnabled(index.isValid());

  if (pageTab!=TMainWnd::DoctorsTab)
    return;

  // для доктора чуть еще расширить... доктор может быть недоступным
  bool available=model->index(index.row(),model->fieldIndex("available")).data().toBool();
  if (available==false)
    makeApptActn->setEnabled(false);
  }

void TMainWnd::setupDataIndex(const QModelIndex &index)
  {
  if (index.isValid()==false)
    {
    adjustDataActions();
    return;
    }

  dataTable->selectRow(index.row());
  dataTable->scrollTo(index,QAbstractItemView::EnsureVisible);

  dataTable->setFocus();
  }

TModelDlg *TMainWnd::dialogByModel(TSqlTableModel *model, const QSqlRecord &record)
  {
  if (model==m_doctorsModel)
    {
    TDoctorDlg *doctorDlg=new TDoctorDlg(record,this);
    connect(doctorDlg,&TDoctorDlg::routinesChanged,this,[this]() {
      m_dataChanged=true;
      });
    // именно QueuedConnection, надо чтобы dataChanged у моделей раньше отработали
    connect(doctorDlg,&TDoctorDlg::showApptsNeeded,this,&TMainWnd::showSheduleAppts,Qt::QueuedConnection);
    return doctorDlg;
    }

  if (model==m_clientsModel)
    return new TClientDlg(record, this);

  if (model==m_petsModel)
    {
    TPetDlg *petDlg=new TPetDlg(record, this);
    connect(petDlg,&TPetDlg::updateDataNeeded,this,&TMainWnd::updateData);
    return petDlg;
    }

  return nullptr;
  }

void TMainWnd::appendData(TSqlTableModel *dataModel)
  {
  if (dataModel==nullptr)
    {
    dataModel=qobject_cast<TSqlTableModel*>(dataTable->model());
    if (dataModel==nullptr)
      return;
    }

  // нужна "чистая" и пустая запись, без Автоинремента и Relations, достаем ее из m_dataModule
  QSqlRecord record=m_dataModule->tableRecord(dataModel->tableName());
  TModelDlg *modelDlg=dialogByModel(dataModel,record);
  if (modelDlg==nullptr)
    return;

  if (modelDlg->exec()!=QDialog::Accepted)
    {
    delete modelDlg;
    return;
    }

  record=modelDlg->record();
  delete modelDlg;

  dataModel->insertRecord(-1,record);
  dataModel->select();

  qint32 id=m_dataModule->lastAutoInc(dataModel->tableName());
  QModelIndexList indexes=dataModel->match(dataModel->index(0,dataModel->fieldIndex("id")),Qt::DisplayRole,id,1,Qt::MatchExactly);
  if (indexes.size()==0)
    return;

  setupDataIndex(indexes.first());
  }

void TMainWnd::editData(TSqlTableModel *dataModel, qint32 row)
  {
  if (dataModel==nullptr)
    {
    dataModel=qobject_cast<TSqlTableModel*>(dataTable->model());
    if (dataModel==nullptr)
      return;
    }

  if (row<0)
    {
    QModelIndexList indexes=dataTable->selectionModel()->selectedRows();
    if (indexes.size()==0)
      return;

    row=indexes.first().row();
    }

  QSqlRecord record=dataModel->record(row);
  TModelDlg *modelDlg=dialogByModel(dataModel,record);
  if (modelDlg==nullptr)
    return;

  if (modelDlg->exec()!=QDialog::Accepted)
    {
    delete modelDlg;
    return;
    }

  record=modelDlg->record();
  delete modelDlg;

  dataModel->setRecord(row,record);
  dataModel->select();

  quint8 column=dataModel->fieldIndex(dataModel->headerField());
  setupDataIndex(dataModel->index(row,column));
  }

void TMainWnd::removeData(TSqlTableModel *dataModel, qint32 row)
  {
  if (dataModel==nullptr)
    {
    dataModel=qobject_cast<TSqlTableModel*>(dataTable->model());
    if (dataModel==nullptr)
      return;
    }

  if (row<0)
    {
    QModelIndexList indexes=dataTable->selectionModel()->selectedRows();
    if (indexes.size()==0)
      return;

    QModelIndex index=indexes.first();
    row=index.row();
    }

  if (QMessageBox::question(this,tr("Confirmation"),tr("Do you really want to remove selected record?"),QMessageBox::Yes | QMessageBox::No)==QMessageBox::No)
    return;

  dataModel->removeRow(row);
  dataModel->select();

  if (row>dataModel->rowCount()-1)
    row=0;

  quint8 column=dataModel->fieldIndex(dataModel->headerField());
  setupDataIndex(dataModel->index(row,column));

  // Заблокировать возможность удаления на 20 сек
  bool removeEnabled=removeDataActn->isEnabled();
  removeDataActn->setEnabled(false);
  QTimer::singleShot(20000,Qt::PreciseTimer, this,[this,removeEnabled]() {
    removeDataActn->setEnabled(removeEnabled);
    });
  }

void TMainWnd::updateData(TSqlTableModel *dataModel, quint32 id)
  {
  if (dataModel==nullptr)
    return;

  QModelIndexList indexes=dataModel->match(dataModel->index(0,dataModel->fieldIndex("id")),Qt::DisplayRole,id,1,Qt::MatchExactly);
  if (indexes.size()>0)
    {
    editData(dataModel,indexes.first().row());
    return;
    }

  appendData(dataModel);
  }

void TMainWnd::dataTableActivated(const QModelIndex &index)
  {
  if (index.isValid()==false)
    return;

  editData(nullptr, -1);
  }

void TMainWnd::dataSelectionChanged(const QItemSelection &selected, const QItemSelection &deselected)
  {
  Q_UNUSED(selected)
  Q_UNUSED(deselected)

  adjustDataActions();
  }

void TMainWnd::setupTaskIndex(const QModelIndex &index)
  {
  if (index.isValid()==false)
    return;

  tasksTable->selectRow(index.row());
  tasksTable->scrollTo(index,QAbstractItemView::EnsureVisible);

  tasksTable->setFocus();
  }

void TMainWnd::appendTask()
  {
  QSqlRecord record=m_dataModule->tableRecord("tasks");
  record.setValue("task_date",QDate::currentDate());

  TTaskDlg taskDlg(record,this);
  if (taskDlg.exec()!=QDialog::Accepted)
    return;

  record=taskDlg.record();
  record.setValue("completed",false);

  m_tasksModel->insertRecord(-1,record);
  m_tasksModel->select();

  qint32 id=m_dataModule->lastAutoInc("tasks");
  QModelIndexList indexes=m_tasksModel->match(m_tasksModel->index(0,m_tasksModel->fieldIndex("id")),Qt::DisplayRole,id,1,Qt::MatchExactly);
  if (indexes.size()==0)
    return;

  setupTaskIndex(indexes.first());
  }

void TMainWnd::editTask()
  {
  QModelIndexList indexes=tasksTable->selectionModel()->selectedRows();
  if (indexes.size()==0)
    return;

  quint32 row=indexes.first().row();
  QSqlRecord record=m_tasksModel->record(row);
  if (record.value("completed").toBool()==true)
    return;

  quint32 id=record.value("id").toUInt();
  TTaskDlg taskDlg(record,this);
  if (taskDlg.exec()!=QDialog::Accepted)
    return;

  record=taskDlg.record();

  m_tasksModel->setRecord(row,record);
  m_tasksModel->select();

  indexes=m_tasksModel->match(m_tasksModel->index(0,m_tasksModel->fieldIndex("id")),Qt::DisplayRole,id,1,Qt::MatchExactly);
  if (indexes.size()==0)
    return;

  setupTaskIndex(indexes.first());
  }

void TMainWnd::removeTask()
  {
  QModelIndexList indexes=tasksTable->selectionModel()->selectedRows();
  if (indexes.size()==0)
    return;

  quint32 row=indexes.first().row();
  QDate taskDate=m_tasksModel->index(row,m_tasksModel->fieldIndex("task_date")).data().toDate();

  QString message=tr("Remove task on the '%1'?").arg(taskDate.toString("dd.MM.yyyy"));
  if (QMessageBox::question(this,tr("Confirmation"),message,QMessageBox::Yes | QMessageBox::No)!=QMessageBox::Yes)
    return;

  m_tasksModel->removeRow(row);
  m_tasksModel->select();

  if (row>m_tasksModel->rowCount()-1)
    row=0;

  setupDataIndex(m_tasksModel->index(row,m_tasksModel->fieldIndex("task_date")));
  }

void TMainWnd::completeTask()
  {
  QModelIndexList indexes=tasksTable->selectionModel()->selectedRows();
  if (indexes.size()==0)
    return;

  quint32 row=indexes.first().row();
  QSqlRecord record=m_tasksModel->record(row);
  if (record.value("completed").toBool()==true)
    return;

  quint32 id=record.value("id").toUInt();

  record.setValue("completed",true);
  m_tasksModel->setRecord(row,record);
  m_tasksModel->select();

  indexes=m_tasksModel->match(m_tasksModel->index(0,m_tasksModel->fieldIndex("id")),Qt::DisplayRole,id,1,Qt::MatchExactly);
  if (indexes.size()==0)
    return;

  setupTaskIndex(indexes.first());
  }

void TMainWnd::tasksTableActivated(const QModelIndex &index)
  {
  Q_UNUSED(index)

  editTask();
  }

void TMainWnd::tasksSelectionChanged(const QItemSelection &selected, const QItemSelection &deselected)
  {
  Q_UNUSED(selected)
  Q_UNUSED(deselected)

  QModelIndexList indexes=tasksTable->selectionModel()->selectedRows();
  bool enabled=indexes.size()>0 && indexes.first().isValid()==true;

  editTaskActn->setEnabled(enabled);
  removeTaskActn->setEnabled(enabled);
  if (enabled==false)
    {
    completeTaskActn->setEnabled(false);
    return;
    }

  bool completed=m_tasksModel->index(indexes.first().row(),m_tasksModel->fieldIndex("completed")).data().toBool();
  completeTaskActn->setEnabled(completed==false);
  }

void TMainWnd::appendReminder()
  {
  TSqlTableModel *dataModel=qobject_cast<TSqlTableModel*>(dataTable->model());
  if (dataModel==nullptr)
    return;

  QModelIndexList indexes=dataTable->selectionModel()->selectedRows();
  if (indexes.size()==0)
    return;

  quint32 row=indexes.first().row();
  quint32 clientId;
  QString clientName;
  switch ((TMainWnd::PageTabs)m_pagesTabBar->currentIndex())
    {
    case TMainWnd::ApptsTab:
      {
      clientId=0;
      break;
      }
    case TMainWnd::PetsTab:
      {
      clientId=dataModel->index(row,dataModel->fieldIndex("client_id")).data().toInt();
      clientName=dataModel->index(row,dataModel->fieldIndex("client_name")).data().toString();
      break;
      }
    case TMainWnd::ClientsTab:
      {
      clientId=dataModel->index(row,dataModel->fieldIndex("id")).data().toInt();
      clientName=dataModel->index(row,dataModel->fieldIndex("name")).data().toString();
      break;
      }
    default:
      return;
    }

  if (clientId==0)
    return;

  // взять запись для таблицы Напоминаний и добавить поле с именем клиента
  QSqlRecord record=m_dataModule->tableRecord("reminders");
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
  record.append(QSqlField("client_name",QMetaType::fromType<QString>(),"reminders"));
#else
  record.append(QSqlField("client_name",QVariant::String,"reminders"));
#endif
  record.setValue("client_name",clientName);
  record.setValue("reminder_date",QDate::currentDate().addDays(1));

  TTaskDlg taskDlg(record,this);
  if (taskDlg.exec()!=QDialog::Accepted)
    return;

  record=taskDlg.record();
  record.setValue("client_id",clientId);

  m_remindersModel->insertRecord(-1,record);
  m_remindersModel->select();

  qint32 id=m_dataModule->lastAutoInc("reminders");
  indexes=m_remindersModel->match(m_remindersModel->index(0,m_remindersModel->fieldIndex("id")),Qt::DisplayRole,id,1,Qt::MatchExactly);
  if (indexes.size()==0)
    return;

  QModelIndex index=indexes.first();
  remindersTable->selectRow(index.row());
  remindersTable->scrollTo(index,QAbstractItemView::EnsureVisible);
  }

void TMainWnd::removeReminder()
  {
  QModelIndexList indexes=remindersTable->selectionModel()->selectedRows();
  if (indexes.size()==0)
    return;

  quint32 row=indexes.first().row();
  QDate reminderDate=m_remindersModel->index(row,m_remindersModel->fieldIndex("reminder_date")).data().toDate();
  QString clientName=m_remindersModel->index(row,m_remindersModel->fieldIndex("client_name")).data().toString();

  QString message=tr("Remove reminder on the '%1' for '%2'?").arg(reminderDate.toString("dd.MM.yyyy"),clientName);
  if (QMessageBox::question(this,tr("Confirmation"),message,QMessageBox::Yes | QMessageBox::No)!=QMessageBox::Yes)
    return;

  m_remindersModel->removeRow(row);
  m_remindersModel->select();
  }

void TMainWnd::remindersTableActivated(const QModelIndex &index)
  {
  QModelIndexList indexes=remindersTable->selectionModel()->selectedRows();
  if (indexes.size()==0)
    return;

  quint32 row=indexes.first().row();
  quint32 clientId=m_remindersModel->index(row,m_remindersModel->fieldIndex("client_id")).data().toUInt();
  if (clientId==0)
    return;

  indexes=m_clientsModel->match(m_clientsModel->index(0,m_clientsModel->fieldIndex("id")),Qt::DisplayRole,clientId,1,Qt::MatchExactly);
  if (indexes.size()==0)
    return;

  m_pagesTabBar->setCurrentIndex((int)TMainWnd::ClientsTab);
  setupDataIndex(indexes.first());
  }

void TMainWnd::remindersSelectionChanged(const QItemSelection &selected, const QItemSelection &deselected)
  {
  Q_UNUSED(selected)
  Q_UNUSED(deselected)

  QModelIndexList indexes=remindersTable->selectionModel()->selectedRows();
  removeRemActn->setEnabled(indexes.size()>0 && indexes.first().isValid()==true);
  }

void TMainWnd::clearDialog()
  {
  dialogEdit->clear();
  }

void TMainWnd::saveDialog()
  {
  if (dialogEdit->toPlainText().isEmpty()==true)
    {
    QMessageBox::critical(this,tr("Error"),tr("Dialog is empty."),QMessageBox::Ok);
    return;
    }

  QString exportPath;
  if (qApp->arguments().contains("-develop")==true)
    exportPath=qApp->applicationDirPath();
  else
    exportPath=QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);

  TSettings settings;
  exportPath=settings.getXmlValue("main_window/export_path","",exportPath).toString();
  QString fileName=QFileDialog::getSaveFileName(this,tr("Save File"),exportPath,tr("HTML files (*.html)"));
  if (fileName.isEmpty()==true)
    return;

  settings.setXmlValue("main_window/export_path","",QFileInfo(fileName).path());

  QFile file(fileName);
  if (file.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text)==false)
    {
    QMessageBox::critical(this,tr("Error"),tr("Can't open file\n%1.").arg(fileName),QMessageBox::Ok);
    return;
    }

  QTextStream stream(&file);
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
  stream.setEncoding(QStringConverter::Utf8);
#else
  stream.setCodec("UTF-8");
#endif
  stream << dialogEdit->document()->toHtml();
  stream.flush();

  file.close();
  }

void TMainWnd::findInDialog()
  {
  QTextCursor selected=dialogEdit->document()->find(searchTextEdit->text(),QTextDocument::FindWholeWords);
  if (selected.isNull()==true)
    return;

  QTextCursor cursor(dialogEdit->document());
  cursor.setPosition(selected.selectionStart());
  cursor.setPosition(selected.selectionEnd(), QTextCursor::KeepAnchor);
  dialogEdit->setTextCursor(cursor);

  dialogEdit->setFocus();
  }

void TMainWnd::findInField()
  {
  QString text=searchDataEdit->text();
  Qt::MatchFlag flags=(Qt::MatchFlag)searchModeCombo->currentData().toInt();
  if (text.isEmpty()==true)
    return;

  TSqlTableModel *model=nullptr;
  quint8 column;
  switch ((TMainWnd::PageTabs)m_pagesTabBar->currentIndex())
    {
    case TMainWnd::PetsTab:
      {
      model=m_petsModel;
      column=model->fieldIndex("name");
      break;
      }
    case TMainWnd::ClientsTab:
      {
      model=m_clientsModel;
      column=model->fieldIndex("name");
      break;
      }
    case TMainWnd::DoctorsTab:
      {
      model=m_doctorsModel;
      column=model->fieldIndex("name");
      break;
      }
    default:
      return;
    }

  QModelIndexList indexes=model->match(model->index(0,column),Qt::DisplayRole,text,1,flags);
  if (indexes.size()==0)
    return;

  QModelIndex index=indexes.first();
  setupDataIndex(index);
  }

void TMainWnd::shedulePeriodChanged(const QDate &date)
  {
  Q_UNUSED(date)

  QDate fromDate=startDateEdit->date()>QDate(2000,1,1) ? startDateEdit->date() : QDate::currentDate();
  QDate toDate=endDateEdit->date()>QDate(2000,1,1) ? endDateEdit->date() : QDate::currentDate().addDays(6);

  if (fromDate>toDate)
    return;

  m_sheduleModel->setPeriod(fromDate,toDate);
  buildShedule();
  }

void TMainWnd::timeSlotsChanged(int index)
  {
  if (index==-1)
    timeslotCombo->setCurrentIndex(0);

  buildShedule();
  }

void TMainWnd::createPetsFilter(int index)
  {
  Q_UNUSED(index)

  if (petsFilterGroup->isChecked()==false)
    {
    m_petsModel->setFilter("");
    return;
    }

  QStringList filterParts;
  if (speciesCombo->currentIndex()>-1)
    filterParts << QString("pets.species_id=%1").arg(speciesCombo->currentData().toInt());

  if (markingCombo->currentIndex()>-1)
    filterParts << QString("pets.marking_id=%1").arg(markingCombo->currentData().toInt());

  if (statusCombo->currentIndex()>-1)
    filterParts << QString("pets.status_id=%1").arg(statusCombo->currentData().toInt());

  QString filter=filterParts.join(" and ");
  m_petsModel->setFilter(filter);
  }

void TMainWnd::petsFilterGroupClicked(bool checked)
  {
  Q_UNUSED(checked)
  createPetsFilter(-1);
  }

void TMainWnd::createDoctorsFilter(int index)
  {
  Q_UNUSED(index)

  if (doctorsFilterGroup->isChecked()==false)
    m_doctorsModel->setFilter("");
  else
    {
    QStringList filterParts;
    if (profileCombo->currentIndex()>-1)
      filterParts << QString("doctors.profile_id=%1").arg(profileCombo->currentData().toInt());

    if (petageCombo->currentIndex()>-1)
      filterParts << QString("doctors.petage_id=%1").arg(petageCombo->currentData().toInt());

    if (actionCombo->currentIndex()>-1)
      filterParts << QString("doctors.action_id=%1").arg(actionCombo->currentData().toInt());

    QString filter=filterParts.join(" and ");
    m_doctorsModel->setFilter(filter);
    }

  // если на вкладке расписание находимся, то сразу перестроить дерево, иначе - пометить, что надо будет перестроить
  if ((TMainWnd::PageTabs)m_pagesTabBar->currentIndex()==TMainWnd::ApptsTab)
    buildShedule();
  else
    m_dataChanged=true;
  }

void TMainWnd::doctorsFilterGroupClicked(bool checked)
  {
  Q_UNUSED(checked)
  createDoctorsFilter(-1);
  }

bool TMainWnd::createApptsFilter()
  {
  TApptDlg apptDlg(TMainWnd::UndefinedTab,QSqlRecord());
  QString filter;
  QStringList filterParts;
  // если не accepted, то возвращать false - по сути не делать экспорт
  if (apptDlg.exec()!=QDialog::Accepted)
    return false;

  QSqlRecord record=apptDlg.record();
  if (record.value("doctor_id").toUInt()>0)
    filterParts << QString("appts.doctor_id=%1").arg(record.value("doctor_id").toUInt());

  if (record.value("client_id").toUInt()>0)
    filterParts << QString("appts.client_id=%1").arg(record.value("client_id").toUInt());

  if (record.value("pet_id").toUInt()>0)
    filterParts << QString("appts.pet_id=%1").arg(record.value("pet_id").toUInt());

  if (record.value("action_id").toUInt()>0)
    filterParts << QString("appts.action_id=%1").arg(record.value("action_id").toUInt());

  QPair<QDate,QDate> period=m_sheduleModel->period();
  filterParts << QString("appt_date between '%1' and '%2'").arg(period.first.toString("yyyy-MM-dd"),period.second.toString("yyyy-MM-dd"));

  filter=filterParts.join(" and ");
  if (doctorsFilterGroup->isChecked()==true)
    {
    // применить фильтр из таблицы докторов
    QString doctorFilter=m_doctorsModel->filter();
    if (doctorFilter.isEmpty()==false)
      filter.append(QString(" and doctor_id in (select doctors.id from doctors where %1)").arg(filter));
    }

  m_apptsModel->setFilter(filter);
  return true;
  }

TMainWnd::ExportData TMainWnd::getExportData()
  {
  TMainWnd::PageTabs pageTab=(TMainWnd::PageTabs)m_pagesTabBar->currentIndex();
  if (pageTab==LlmTab)
    return TMainWnd::ExportData();

  TSqlTableModel *dataModel;

  QStringList headers;
  QStringList fields;
  if (pageTab==TMainWnd::ApptsTab)
    {
    dataModel=m_apptsModel;
    headers=QStringList{tr("Date"),tr("Time"),tr("Action"),tr("Doctor name"),tr("Profile"),tr("Client name"),tr("Pet name"),tr("Species"),tr("Breed")};
    fields=QStringList{"appt_date","appt_time","action","doctor_name","profile","client_name","pet_name","species","breed"};

    // если createApptsFilter() вернул false, вернуть пустую структуру - не делать экспорт по сути
    if (createApptsFilter()==false)
      return TMainWnd::ExportData();
    }
  else
    {
    dataModel=qobject_cast<TSqlTableModel*>(dataTable->model());
    if (dataModel==nullptr)
      return TMainWnd::ExportData();

    QSqlRecord record=dataModel->record();
    // определить данные, которые нужны для экспорта из первоисточников
    for (quint8 column=0;column<record.count();column++)
      {
      if (dataTable->isColumnHidden(column)==true)
        continue;

      headers << dataModel->headerData(column,Qt::Horizontal).toString();
      fields << record.fieldName(column);
      }
    }

  if (dataModel->rowCount()==0)
    {
    QMessageBox::critical(this,tr("Error"),tr("Nothing to export.\nList is empty."),QMessageBox::Ok);
    return TMainWnd::ExportData();
    }

  return TMainWnd::ExportData(dataModel,headers,fields);
  }

void TMainWnd::exportToCsv()
  {
  TMainWnd::ExportData exportData=getExportData();
  if (exportData.model==nullptr)
    {
    m_apptsModel->setFilter(""); // сбросить фильтр для m_apptsModel
    return;
    }

  TSqlTableModel *model=exportData.model;
  QStringList headers=exportData.headers;
  QStringList fields=exportData.fields;
  QString caption=pageTabText((TMainWnd::PageTabs)m_pagesTabBar->currentIndex());

  // выбор файла для экспорта
  QString exportPath;
  if (qApp->arguments().contains("-develop")==true)
    exportPath=qApp->applicationDirPath();
  else
    exportPath=QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);

  TSettings settings;
  exportPath=settings.getXmlValue("main_window/export_path","",exportPath).toString()+"/"+caption;

  QString fileName=QFileDialog::getSaveFileName(this,tr("Save CSV file"),exportPath,tr("CSV files (*.csv)"));
  if (fileName.isEmpty()==true)
    {
    m_apptsModel->setFilter(""); // сбросить фильтр для m_apptsModel
    return;
    }

  settings.setXmlValue("main_window/export_path","",QFileInfo(fileName).path());

  QFile csvFile(fileName);
  if (csvFile.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text)==false)
    {
    QMessageBox::critical(this,tr("Error"),tr("Can't open file\n%1.").arg(fileName));
    m_apptsModel->setFilter(""); // сбросить фильтр для m_apptsModel
    return;
    }

  QTextStream csvStream(&csvFile);
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
  csvStream.setEncoding(QStringConverter::Utf8);
#else
  csvStream.setCodec("UTF-8");
#endif

  // первой строкой отправить заголовки в stream
  csvStream << headers.join(";") << "\n";

  QStringList dateFields={"appt_date", "birth_date", "marking_date", "certificate_date"};
  QStringList timeFields={"appt_time"};
  for (quint32 row=0;row<model->rowCount();row++)
    {
    QStringList values;
    QSqlRecord record=model->record(row);
    for (quint8 column=0;column<fields.size();column++)
      {
      if (fields.at(column)=="gender")
        {
        values << (record.value(fields.at(column)).toBool()==false ? tr("Female") : tr("Male"));
        continue;
        }

      if (dateFields.contains(fields.at(column))==true)
        {
        values << record.value(fields.at(column)).toDate().toString("dd.MM.yyyy");
        continue;
        }

      if (timeFields.contains(fields.at(column))==true)
        {
        values << record.value(fields.at(column)).toTime().toString("hh:mm");
        continue;
        }

      values << record.value(fields.at(column)).toString();
      }

    // каждую строку с данными отправить в stream
    csvStream << values.join(";") << "\n";
    }

  csvStream.flush();
  csvFile.close();

  m_apptsModel->setFilter(""); // сбросить фильтр для m_apptsModel
  }

void TMainWnd::exportToXml()
  {
  TMainWnd::ExportData exportData=getExportData();
  if (exportData.model==nullptr)
    {
    m_apptsModel->setFilter(""); // сбросить фильтр для m_apptsModel
    return;
    }

  TSqlTableModel *model=exportData.model;
  QStringList headers=exportData.headers;
  QStringList fields=exportData.fields;
  QString caption=pageTabText((TMainWnd::PageTabs)m_pagesTabBar->currentIndex());

  // выбор файла для экспорта
  QString exportPath;
  if (qApp->arguments().contains("-develop")==true)
    exportPath=qApp->applicationDirPath();
  else
    exportPath=QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);

  TSettings settings;
  exportPath=settings.getXmlValue("main_window/export_path","",exportPath).toString()+"/"+caption;

  QString fileName=QFileDialog::getSaveFileName(this,tr("Save XML file"),exportPath,tr("XML files (*.xml)"));
  if (fileName.isEmpty()==true)
    {
    m_apptsModel->setFilter(""); // сбросить фильтр для m_apptsModel
    return;
    }

  settings.setXmlValue("main_window/export_path","",QFileInfo(fileName).path());

  QFile xmlFile(fileName);
  if (xmlFile.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text)==false)
    {
    QMessageBox::critical(this,tr("Error"),tr("Can't open file\n%1.").arg(fileName));
    m_apptsModel->setFilter(""); // сбросить фильтр для m_apptsModel
    return;
    }

  xmlFile.close();

  QStringList dateFields={"appt_date", "birth_date", "marking_date", "certificate_date"};
  QStringList timeFields={"appt_time"};

  QList<QMap<QString,QVariant>> values;
  for (quint32 row=0; row<model->rowCount(); row++)
    {
    QMap<QString,QVariant> map;
    QSqlRecord record=model->record(row);
    for (quint8 column=0;column<fields.size();column++)
      {
      // в XML имена атрибутов не могут содержать пробелы;
      QString header=headers.at(column);
      header.replace(" ","_");
      if (fields.at(column)=="gender")
        {
        map[header]=record.value(fields.at(column)).toBool()==false ? tr("Female") : tr("Male");
        continue;
        }

      if (dateFields.contains(fields.at(column))==true)
        {
        map[header]=record.value(fields.at(column)).toDate().toString("dd.MM.yyyy");
        continue;
        }

      if (timeFields.contains(fields.at(column))==true)
        {
        map[header]=record.value(fields.at(column)).toTime().toString("hh:mm");
        continue;
        }

      map[header]=record.value(fields.at(column));
      }

    values << map;
    }

  TSettings xmlSettings(fileName,caption);
  xmlSettings.setXmlMaps("record",true,values);

  m_apptsModel->setFilter(""); // сбросить фильтр для m_apptsModel
  }

void TMainWnd::makeSettings()
  {
  TSettingsDlg settingsDlg;
  connect(&settingsDlg,&TSettingsDlg::setTheme,this,&TMainWnd::setAppTheme);
  if (settingsDlg.exec()!=QDialog::Accepted)
    return;

  TSettings settings;
  QFont font=settings.getXmlValue("main_window/font","",qApp->font()).value<QFont>();
  if (font!=qApp->font())
    setAppFont(font);

  quint16 deadline=settings.getXmlValue("tasks_dock/reminder_deadline","",7).toUInt();
  if (deadline!=m_tasksDelegate->deadline())
    m_tasksDelegate->setReminderDeadline(deadline);
  }

void TMainWnd::checkArchiveNeeded()
  {
  // первый раз метод вызывается непосредственно из конструктора, потом по таймеру и сендер будет таймер
  bool firstStart=qobject_cast<QTimer*>(sender())==nullptr;

  TSettings settings;
  // если вообще надо делать автоархивы
  if (settings.getXmlValue("archiving/enable_auto","",false).toBool()==true)
    {
    quint16 archPeriod=settings.getXmlValue("archiving/period","",30).toUInt();
    QDateTime archDateTime=QDateTime::fromSecsSinceEpoch(settings.getXmlValue("archiving/date_time","",0).toUInt());
    QDateTime dateTime=QDateTime::currentDateTime();
    bool midnight=dateTime.time().hour()==0 && dateTime.time().minute()<30;

    // если первый запуск или полночь и время делать архив
    if (archDateTime.addDays(archPeriod)>=dateTime && (firstStart==true || midnight))
      makeArchive();
    }

  if (settings.getXmlValue("archiving/enable_alarm","",false).toBool()==true)
    {
    QFileInfo info(m_dataModule->baseName());
    if ((double)info.size()/1024/1024 > settings.getXmlValue("archiving/alarm_size","",30).toUInt())
      QMessageBox::warning(this, tr("Attention"),tr("Database size excceds %1 MB.\n"
                                                    "You can alarm change value in Settings.\n"
                                                    "Menu (Option -> Settings)"), QMessageBox::Ok);
    }
  }

void TMainWnd::makeArchive()
  {
  // надо знать ручное архивирование или автоматическое
  bool autoArchive=qobject_cast<QAction*>(sender())==nullptr;

  QString databaseName=m_dataModule->baseName();
  QString folderName=QFileInfo(databaseName).absolutePath()+"/archive";
  QDateTime dateTime=QDateTime::currentDateTime();
  QString archiveName=folderName+QString("/petcore_%1.sqt").arg(dateTime.toString("dd_MM_yyyy-hh_mm_ss"));

  QDir dir;
  if (dir.exists(folderName)==false)
    dir.mkpath(folderName);

  if (QFile::copy(databaseName,archiveName)==false)
    {
    if (autoArchive==false)
      QMessageBox::critical(this,tr("Error"),tr("Archive hasn't been created."),QMessageBox::Ok);

    return;
    }

  if (autoArchive==false)
    QMessageBox::information(this,tr("Information"),tr("Archive has been successfully created with name\n%1.").arg(archiveName),QMessageBox::Ok);

  TSettings settings;
  settings.setXmlValue("archiving/date_time","",dateTime.toSecsSinceEpoch());

  quint16 tasksDays=settings.getXmlValue("tasks_dock/task_lifetime","",7).toUInt();
  quint16 remindersDays=settings.getXmlValue("tasks_dock/reminder_deadline","",7).toUInt();
  quint16 apptsDays=settings.getXmlValue("archiving/keep_past","",99).toUInt();

  QSqlQuery query(m_dataModule->database());

  // определить дату (вычесть tasksDays), и удалить из tasks все записи старше этой даты
  query.exec(QString("delete from tasks where completed=true or task_date<'%1'").arg(QDate::currentDate().addDays(-1*tasksDays).toString("yyyy-MM-dd")));
  qint32 rows=query.numRowsAffected();
  if (rows!=0)
    m_tasksModel->select();

  // определить дату (вычесть remDays), и удалить из reminders все записи старше этой даты
  query.exec(QString("delete from reminders where reminder_date<'%1'").arg(QDate::currentDate().addDays(-1*remindersDays).toString("yyyy-MM-dd")));
  rows=query.numRowsAffected();
  if (rows!=0)
    m_remindersModel->select();

  // определить дату (вычесть pastDays), и удалить из appts все записи старше этой даты
  query.exec(QString("delete from appts where appt_date<'%1'").arg(QDate::currentDate().addDays(-1*apptsDays).toString("yyyy-MM-dd")));
  rows=query.numRowsAffected();
  // обновить m_apptModel и перестроить Расписание, если rows!=0 (-1 - не получилось определить, на всякий случай тоже обновить и перестроить)
  if (rows!=0)
    {
    m_apptsModel->select();
    buildShedule();
    }
  }



void TMainWnd::processAuthorization()
  {
  TLoginDlg loginDlg;
  if (loginDlg.exec()!=QDialog::Accepted)
    {
    qApp->quit();
    return;
    }

  QPair<QString,QString> credentials=loginDlg.credentials();

  createWaitDlg(tr("Please wait until token received."));
  QJsonObject response=m_petcoreApiWorker->sendApiRequest("Authorize",{{"login", credentials.first}, {"password", credentials.second}});
  destroyWaitDlg();

  qint16 code=response.value("code").toInt();
  quint16 errorCode=response.value("error_code").toInt();
  QString errorText=response.value("error_string").toString();
  if (errorCode!=QNetworkReply::NoError || code!=200)
    {
    QMessageBox::critical(this,tr("Error"),tr("Can't access authorixation server.\n%1\nTry again please.").arg(errorText),QMessageBox::Ok);
    QTimer::singleShot(100,this,&TMainWnd::processAuthorization);
    return;
    }

  QJsonObject result=response.value("result").toObject();
  if (result.value("code").toInt()!=1 || result.value("status").toString()!="success")
    {
    QMessageBox::critical(this,tr("Error"),tr("Invalid email or password.").arg(errorText),QMessageBox::Ok);
    QTimer::singleShot(100,this,&TMainWnd::processAuthorization);
    return;
    }

  QByteArray token=response.value("token").toString().toLocal8Bit();
  QJsonObject user=response.value("user").toObject();
  quint32 clinicId=user.value("id").toInt();
  QString clinicTitle=user.value("title").toString();

  QByteArray message=QByteArrayLiteral("Web_Success_Petcore");
  message=QCryptographicHash::hash(message, QCryptographicHash::Sha256);

  token=QAESEncryption::Crypt(QAESEncryption::AES_256, QAESEncryption::ECB, token, message);
  token=token.toHex();

  TSettings settings;
  settings.setXmlValue("clinic/id","",clinicId);
  settings.setXmlValue("clinic/title","",clinicTitle);
  settings.setXmlValue("petcore_api/access_token","",QString(token));

  setWindowTitle(windowTitle()+QString(" [%1]").arg(clinicTitle));
  m_petcoreApiWorker->initialize("petcore_api");

  loadDictionaries();
  }

void TMainWnd::checkToken()
  {
  QJsonObject response=m_petcoreApiWorker->sendApiRequest("CheckToken",{});
  qint16 code=response.value("code").toInt();
  quint16 errorCode=response.value("error_code").toInt();
  if (errorCode!=QNetworkReply::NoError || code!=200)
    {
    QTimer::singleShot(600000,this,&TMainWnd::checkToken);
    return;
    }

  if (response.value("result").toBool()==true)
    {
    QTimer::singleShot(86400000,this,&TMainWnd::checkToken);
    return;
    }

  QMessageBox::critical(this,tr("Error"),tr("You don't have the right to use PetCore software."),QMessageBox::Ok);
  processAuthorization();
  }

void TMainWnd::sendTelegram()
  {
  QString recipient;
  QString telegram;
  QString phoneNumber;

  // в заисимости от того, в какой вкладке находимся, цепляем нужные данные
  if ((TMainWnd::PageTabs)m_pagesTabBar->currentIndex()==TMainWnd::ApptsTab)
    {
    QModelIndexList indexes=sheduleTree->selectionModel()->selectedRows();
    if (indexes.size()==0)
      return;

    QModelIndex sheduleIndex=indexes.first();
    TSheduleModel::Types type=(TSheduleModel::Types)m_sheduleModel->index(sheduleIndex.row(),TSheduleModel::Type,sheduleIndex.parent()).data().toUInt();
    if (type==TSheduleModel::TimeType)
      {
      // в DataRecord лежат данные из TApptsModel
      QHash<QString,QVariant> apptHash=m_sheduleModel->index(sheduleIndex.row(),TSheduleModel::DataRecord,sheduleIndex.parent()).data().toHash();
      recipient=apptHash.value("client_name").toString();
      telegram=apptHash.value("telegram").toString();
      phoneNumber=apptHash.value("phone_number").toString();
      }
    else
      {
      // в DataRecord лежат данные из TRoutinesModel
      QHash<QString,QVariant> routineHash=m_sheduleModel->index(sheduleIndex.row(),TSheduleModel::DataRecord,sheduleIndex.parent()).data().toHash();
      recipient=routineHash.value("name").toString();
      telegram=routineHash.value("telegram").toString();
      phoneNumber=routineHash.value("phone_number").toString();
      }
    }
  else
    {
    TSqlTableModel *dataModel=qobject_cast<TSqlTableModel*>(dataTable->model());
    if (dataModel==nullptr)
      return;

    QModelIndexList indexes=dataTable->selectionModel()->selectedRows();
    if (indexes.size()==0)
      return;

    QModelIndex index=indexes.first();
    switch ((TMainWnd::PageTabs)m_pagesTabBar->currentIndex())
      {
      case TMainWnd::PetsTab:
        {
        recipient=dataModel->index(index.row(),dataModel->fieldIndex("client_name")).data().toString();
        telegram=dataModel->index(index.row(),dataModel->fieldIndex("telegram")).data().toString();
        phoneNumber=dataModel->index(index.row(),dataModel->fieldIndex("phone_number")).data().toString();
        break;
        }
      case TMainWnd::ClientsTab:
        {
        recipient=dataModel->index(index.row(),dataModel->fieldIndex("name")).data().toString();
        telegram=dataModel->index(index.row(),dataModel->fieldIndex("telegram")).data().toString();
        phoneNumber=dataModel->index(index.row(),dataModel->fieldIndex("phone_number")).data().toString();
        break;
        }
      case TMainWnd::DoctorsTab:
        {
        recipient=dataModel->index(index.row(),dataModel->fieldIndex("name")).data().toString();
        telegram=dataModel->index(index.row(),dataModel->fieldIndex("telegram")).data().toString();
        phoneNumber=dataModel->index(index.row(),dataModel->fieldIndex("phone_number")).data().toString();
        break;
        }
      default:
        return;
      }
    }

  bool accepted;
  QString text=QInputDialog::getMultiLineText(this,tr("Telegram message"),tr("Message for '%1'").arg(recipient),QString(),&accepted);
  if (text.isEmpty()==true || accepted==false)
    return;

  QJsonObject response=m_petcoreApiWorker->sendApiRequest("SendTelegram",{{"telegram", telegram},{"phone_number", phoneNumber},
                                                                          {"name", recipient},{"message", text}});

  qint16 code=response.value("code").toInt();
  quint16 errorCode=response.value("error_code").toInt();
  QString errorText=response.value("error_string").toString();
  if (errorCode!=QNetworkReply::NoError || code!=200)
    {
    QMessageBox::critical(this,tr("Error"),tr("Can't access server.\n%1.").arg(errorText),QMessageBox::Ok);
    return;
    }

  QJsonObject result=response.value("result").toObject();
  if (result.value("code").toInt()!=1 || result.value("status").toString()!="success")
    {
    QMessageBox::critical(this,tr("Error"),tr("Can't send telegram message."),QMessageBox::Ok);
    return;
    }

  if (response.value("send_status").toBool()==false)
    {
    QMessageBox::critical(this,tr("Error"),tr("Telegram message hasn't been send."),QMessageBox::Ok);
    return;
    }

  QMessageBox::information(this,tr("Information"),tr("Telegram message has been successfully send."),QMessageBox::Ok);
  }

void TMainWnd::loadDictionaries()
  {
  createSplashScreen(tr("Please wait while the data is being loaded from the server:"));

  /*QStringList dots={".","..","...","...."};
  quint8 frame=0;
  QTimer timer;
  timer.setTimerType(Qt::PreciseTimer);
  timer.setInterval(300);
  connect(&timer,&QTimer::timeout,this,[this,dots,&frame]() {
    showSplashMessage(tr("Please wait while the data is being loaded from the server").append(dots.at(frame++)));
    if (frame>dots.size()-1)
      frame=0;
    });

  timer.start();*/

  QSqlQuery query(m_dataModule->database());

  showSplashMessage(tr("Please wait while the data is being loaded from the server:"),tr("Species..."));
  QJsonObject response=m_petcoreApiWorker->sendApiRequest("GetSpecies",{});

  qint16 code=response.value("code").toInt();
  quint16 errorCode=response.value("error_code").toInt();
  //виды животных
  if (errorCode==QNetworkReply::NoError || code==200)
    {
    QVariantList ids;
    QVariantList species;
    QVariantList forbiddens;

    QJsonArray array=response.value("data").toArray();
    for (const QJsonValue &value: array)
      {
      QJsonObject object=value.toObject();

      ids << object.value("id").toInt();
      species << object.value("title").toString();
      forbiddens << object.value("forbidden").toBool();
      }

    // очистить таблицу Видов
    query.exec("delete from species");

    // толкнуть в таблицу Видов полученные данные, insert or update - на случай, если почему-то не очистилась таблица
    query.prepare("insert or replace into species(id,species,forbidden) values(?,?,?)");
    query.addBindValue(ids);
    query.addBindValue(species);
    query.addBindValue(forbiddens);

    query.execBatch(QSqlQuery::ValuesAsRows);
    m_speciesModel->select();
    }

  // породы
  showSplashMessage(tr("Please wait while the data is being loaded from the server:"),tr("Breeds..."));
  response=m_petcoreApiWorker->sendApiRequest("GetBreeds",{{"species_id",0}});
  code=response.value("code").toInt();
  errorCode=response.value("error_code").toInt();
  if (errorCode==QNetworkReply::NoError || code==200)
    {
    QVariantList ids;
    QVariantList speciesIds;
    //QVariantList subSpeciesIds;
    QVariantList breeds;
    QVariantList forbiddens;

    QJsonArray array=response.value("data").toArray();
    for (const QJsonValue &value: array)
      {
      QJsonObject object=value.toObject();

      ids << object.value("id").toInt();
      speciesIds << object.value("animal_id").toInt();
      //subSpeciesIds << QString();
      breeds << object.value("title").toString();
      forbiddens << object.value("forbidden").toBool();
      }

    // очистить таблицу Пород
    query.exec("delete from breeds");

    // толкнуть в таблицу Пород полученные данные, insert or update - на случай, если почему-то не очистилась таблица
    query.prepare("insert or replace into breeds(id,species_id,breed,forbidden) values(?,?,?,?)");
    query.addBindValue(ids);
    query.addBindValue(speciesIds);
    //query.addBindValue(subSpeciesIds);
    query.addBindValue(breeds);
    query.addBindValue(forbiddens);

    query.execBatch(QSqlQuery::ValuesAsRows);

    TBreedsModel *breedsModel=qobject_cast<TBreedsModel*>(m_dataModule->tableModel("breeds"));
    if (breedsModel!=nullptr)
      breedsModel->select();
    }

  // статусы животных
  showSplashMessage(tr("Please wait while the data is being loaded from the server:"),tr("Statuses..."));
  response=m_petcoreApiWorker->sendApiRequest("GetStatuses",{});
  code=response.value("code").toInt();
  errorCode=response.value("error_code").toInt();
  if (errorCode==QNetworkReply::NoError || code==200)
    {
    QVariantList ids;
    QVariantList statuses;

    QJsonArray array=response.value("data").toArray();
    for (const QJsonValue &value: array)
      {
      QJsonObject object=value.toObject();

      ids << object.value("id").toInt();
      statuses << object.value("title").toString();
      }

    // очистить таблицу статусов
    query.exec("delete from statuses");

    // толкнуть в таблицу Статусов полученные данные, insert or update - на случай, если почему-то не очистилась таблица
    query.prepare("insert or replace into statuses(id,status) values(?,?)");
    query.addBindValue(ids);
    query.addBindValue(statuses);

    query.execBatch(QSqlQuery::ValuesAsRows);
    m_statusesModel->select();
    }

  // типы маркировок
  showSplashMessage(tr("Please wait while the data is being loaded from the server:"),tr("Marking typies..."));
  response=m_petcoreApiWorker->sendApiRequest("GetMarking",{});
  code=response.value("code").toInt();
  errorCode=response.value("error_code").toInt();
  if (errorCode==QNetworkReply::NoError || code==200)
    {
    QVariantList ids;
    QVariantList marking;

    QJsonArray array=response.value("data").toArray();
    for (const QJsonValue &value: array)
      {
      QJsonObject object=value.toObject();

      ids << object.value("id").toInt();
      marking << object.value("title").toString();
      }

    // очистить таблицу профилей
    query.exec("delete from marking");

    // толкнуть в таблицу Профилей полученные данные, insert or update - на случай, если почему-то не очистилась таблица
    query.prepare("insert or replace into marking(id,marking) values(?,?)");
    query.addBindValue(ids);
    query.addBindValue(marking);

    query.execBatch(QSqlQuery::ValuesAsRows);
    m_markingModel->select();
    }

  // профили врачей
  showSplashMessage(tr("Please wait while the data is being loaded from the server:"),tr("Profiles..."));
  response=m_petcoreApiWorker->sendApiRequest("GetProfiles",{});
  code=response.value("code").toInt();
  errorCode=response.value("error_code").toInt();
  if (errorCode==QNetworkReply::NoError || code==200)
    {
    QVariantList ids;
    QVariantList profiles;

    QJsonArray array=response.value("data").toArray();
    for (const QJsonValue &value: array)
      {
      QJsonObject object=value.toObject();

      ids << object.value("id").toInt();
      profiles << object.value("title").toString();
      }

    // очистить таблицу профилей
    query.exec("delete from profiles");

    // толкнуть в таблицу Профилей полученные данные, insert or update - на случай, если почему-то не очистилась таблица
    query.prepare("insert or replace into profiles(id,profile) values(?,?)");
    query.addBindValue(ids);
    query.addBindValue(profiles);

    query.execBatch(QSqlQuery::ValuesAsRows);
    m_profilesModel->select();
    }

  // ученые степени врачей
  showSplashMessage(tr("Please wait while the data is being loaded from the server:"),tr("Degrees..."));
  response=m_petcoreApiWorker->sendApiRequest("GetDegrees",{});
  code=response.value("code").toInt();
  errorCode=response.value("error_code").toInt();
  if (errorCode==QNetworkReply::NoError || code==200)
    {
    QVariantList ids;
    QVariantList degrees;

    QJsonArray array=response.value("data").toArray();
    for (const QJsonValue &value: array)
      {
      QJsonObject object=value.toObject();

      ids << object.value("id").toInt();
      degrees << object.value("title").toString();
      }

    // очистить таблицу степеней
    query.exec("delete from degrees");

    // толкнуть в таблицу Профилей полученные данные, insert or update - на случай, если почему-то не очистилась таблица
    query.prepare("insert or replace into degrees(id,degree) values(?,?)");
    query.addBindValue(ids);
    query.addBindValue(degrees);

    query.execBatch(QSqlQuery::ValuesAsRows);

    TDegreesModel *degreesModel=qobject_cast<TDegreesModel*>(m_dataModule->tableModel("degree"));
    if (degreesModel!=nullptr)
      degreesModel->select();
    }

  // ученые звания врачей
  showSplashMessage(tr("Please wait while the data is being loaded from the server:"),tr("Ranks..."));
  response=m_petcoreApiWorker->sendApiRequest("GetRanks",{});
  code=response.value("code").toInt();
  errorCode=response.value("error_code").toInt();
  if (errorCode==QNetworkReply::NoError || code==200)
    {
    QVariantList ids;
    QVariantList ranks;

    QJsonArray array=response.value("data").toArray();
    for (const QJsonValue &value: array)
      {
      QJsonObject object=value.toObject();

      ids << object.value("id").toInt();
      ranks << object.value("title").toString();
      }

    // очистить таблицу степеней
    query.exec("delete from ranks");

    // толкнуть в таблицу Профилей полученные данные, insert or update - на случай, если почему-то не очистилась таблица
    query.prepare("insert or replace into ranks(id,rank) values(?,?)");
    query.addBindValue(ids);
    query.addBindValue(ranks);

    query.execBatch(QSqlQuery::ValuesAsRows);

    TRanksModel *ranksModel=qobject_cast<TRanksModel*>(m_dataModule->tableModel("ranks"));
    if (ranksModel!=nullptr)
      ranksModel->select();
    }

  // возрастные группы животных
  showSplashMessage(tr("Please wait while the data is being loaded from the server:"),tr("Pet ages..."));
  response=m_petcoreApiWorker->sendApiRequest("GetPetages",{});
  code=response.value("code").toInt();
  errorCode=response.value("error_code").toInt();
  if (errorCode==QNetworkReply::NoError || code==200)
    {
    QVariantList ids;
    QVariantList petages;

    QJsonArray array=response.value("data").toArray();
    for (const QJsonValue &value: array)
      {
      QJsonObject object=value.toObject();

      ids << object.value("id").toInt();
      petages << object.value("title").toString();
      }

    // очистить таблицу степеней
    query.exec("delete from petages");

    // толкнуть в таблицу Профилей полученные данные, insert or update - на случай, если почему-то не очистилась таблица
    query.prepare("insert or replace into petages(id,pet_age) values(?,?)");
    query.addBindValue(ids);
    query.addBindValue(petages);

    query.execBatch(QSqlQuery::ValuesAsRows);
    m_petagesModel->select();
    }

  // процедуры или действия
  showSplashMessage(tr("Please wait while the data is being loaded from the server:"),tr("Actions..."));
  response=m_petcoreApiWorker->sendApiRequest("GetActions",{});
  code=response.value("code").toInt();
  errorCode=response.value("error_code").toInt();
  if (errorCode==QNetworkReply::NoError || code==200)
    {
    QVariantList ids;
    QVariantList actions;

    QJsonArray array=response.value("data").toArray();
    for (const QJsonValue &value: array)
      {
      QJsonObject object=value.toObject();

      ids << object.value("id").toInt();
      actions << object.value("title").toString();
      }

    // очистить таблицу степеней
    query.exec("delete from actions");

    // толкнуть в таблицу Профилей полученные данные, insert or update - на случай, если почему-то не очистилась таблица
    query.prepare("insert or replace into actions(id,action) values(?,?)");
    query.addBindValue(ids);
    query.addBindValue(actions);

    query.execBatch(QSqlQuery::ValuesAsRows);
    m_actionsModel->select();
    }

  //timer.stop();
  destroySplashScreen();
  }

void TMainWnd::sendLlmQuery()
  {
  QString text=questionEdit->text();
  if (text.isEmpty()==true)
    return;

  QString prefix=tr("<b>[%1] You:</b><br>").arg(QDateTime::currentDateTime().toString("dd.MM.yyyy hh:mm:ss"));
  QString htmlText=text.replace("<","&lt;").replace(">","&gt;").replace("\n","<br>");
  dialogEdit->appendHtml("<font color='#006000'>"+prefix+htmlText+"</font><br>");

  //m_qwenApiWorker->sendApiRequest("Question",{{"prompt",tr("You are the veterinarian and your name is PetCore")},{"question",text}},false);
  m_gigachatApiWorker->sendApiRequest("Question",{{"prompt",tr("You are the veterinarian and your name is PetCore")},{"question",text}},false);
  askBtn->setEnabled(false);
  questionEdit->clear();
  }

void TMainWnd::llmAnswerReceived(const QJsonObject &response)
  {
  // NOTE: цвета так нельзя, надо иные!!
  askBtn->setEnabled(true);
  QString prefix=tr("<b>[%1] Veterinarian:</b>").arg(QDateTime::currentDateTime().toString("dd.MM.yyyy hh:mm:ss"));
  QJsonArray answers=response.value("choices").toArray();
  if (answers.size()==0)
    {
    prefix.append("<br>");
    QString errorString=response.value("error_string").toString();
    QString text=errorString.isEmpty()==true ? tr("No answer") : errorString;

    dialogEdit->appendHtml("<font color='#800000'>"+prefix+text+"</font><br>");
    return;
    }

  QStringList texts;
  for (quint8 i=0;i<answers.size();i++) // варианты ответов, их может быть несколько, см. параметр n в запросе
    {
    QJsonObject message=answers.at(i).toObject().value("message").toObject();
    texts.append(message.value("content").toString());
    }

  QTextDocument markdown;
  markdown.setMarkdown(texts.join("\n"));
  QString htmlText=markdown.toHtml();

  // обрезать ненужные мне заголовки...
  int bodyStart=htmlText.indexOf("<p style");
  int bodyEnd=htmlText.lastIndexOf("</p>");  connect(qApp,&QApplication::aboutToQuit,this,&TMainWnd::aboutToQuit);
  htmlText=htmlText.mid(bodyStart,bodyEnd-bodyStart+QString("</p>").size());

  dialogEdit->appendHtml("<font color='#000080'>"+prefix+htmlText+"</font>");
  dialogEdit->appendHtml(""); // пустую строку добавить, "<br>" в конце делает два переноса, вероятно из-за "<p style...>...</p>"
  }
