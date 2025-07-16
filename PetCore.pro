TEMPLATE = app
TARGET = PetCore

CONFIG += c++17

QT += core \
      gui \
      widgets \
      xml \
      sql \
      network
    
FORMS += ui/TMainWnd.ui \
         ui/TApptDlg.ui \
         ui/TPrefferedDlg.ui \
         ui/TClientDlg.ui \
         ui/TAboutDlg.ui \
         ui/TDoctorDlg.ui \
         ui/TLoginDlg.ui \
         ui/TPeriodDlg.ui \
         ui/TPetDlg.ui \
         ui/TReminderDlg.ui \
         ui/TSettingsDlg.ui

HEADERS += src/TMainWnd.h \
           src/3rdparty/QAesEncryption/QAesEncryption.h \
           src/TAboutDlg.h \
           src/TActionsModel.h \
           src/TApptDlg.h \
           src/TApptsFilterModel.h \
           src/TApptsModel.h \
           src/TBreedsModel.h \
           src/TClientDlg.h \
           src/TClientsModel.h \
           src/TComboBox.h \
           src/TDadataApiWorker.h \
           src/TDataDelegate.h \
           src/TDataModule.h \
           src/TDegreesModel.h \
           src/TDoctorDlg.h \
           src/TDoctorsModel.h \
           src/TGigachatApiWorker.h \
           src/TLoginDlg.h \
           src/TMarkingModel.h \
           src/TModelDlg.h \
           src/TPasswordEdit.h \
           src/TPeriodDlg.h \
           src/TPetDlg.h \
           src/TPetagesModel.h \
           src/TPetcoreApiWorker.h \
           src/TPetsModel.h \
           src/TPrefferedDlg.h \
           src/TProfilesModel.h \
           src/TQwenApiWorker.h \
           src/TRanksModel.h \
           src/TReminderDlg.h \
           src/TRemindersDelegate.h \
           src/TRemindersModel.h \
           src/TRestApiWorker.h \
           src/TRoutinesDelegate.h \
           src/TRoutinesFilterModel.h \
           src/TRoutinesModel.h \
           src/TSettings.h \
           src/TSettingsDlg.h \
           src/TSheduleDelegate.h \
           src/TSheduleModel.h \
           src/TSpeciesModel.h \
           src/TSqlRelation.h \
           src/TSqlTableModel.h \
           src/TStatusesModel.h \
           src/TSubspeciesModel.h

SOURCES += src/main.cpp \
           src/3rdparty/QAesEncryption/QAesEncryption.cpp \
           src/TAboutDlg.cpp \
           src/TActionsModel.cpp \
           src/TApptDlg.cpp \
           src/TApptsFilterModel.cpp \
           src/TApptsModel.cpp \
           src/TBreedsModel.cpp \
           src/TClientDlg.cpp \
           src/TClientsModel.cpp \
           src/TComboBox.cpp \
           src/TDadataApiWorker.cpp \
           src/TDataDelegate.cpp \
           src/TDataModule.cpp \
           src/TDegreesModel.cpp \
           src/TDoctorDlg.cpp \
           src/TDoctorsModel.cpp \
           src/TGigachatApiWorker.cpp \
           src/TLoginDlg.cpp \
           src/TMainWnd.cpp \
           src/TMarkingModel.cpp \
           src/TModelDlg.cpp \
           src/TPasswordEdit.cpp \
           src/TPeriodDlg.cpp \
           src/TPetDlg.cpp \
           src/TPetagesModel.cpp \
           src/TPetcoreApiWorker.cpp \
           src/TPetsModel.cpp \
           src/TPrefferedDlg.cpp \
           src/TProfilesModel.cpp \
           src/TQwenApiWorker.cpp \
           src/TRanksModel.cpp \
           src/TReminderDlg.cpp \
           src/TRemindersDelegate.cpp \
           src/TRemindersModel.cpp \
           src/TRestApiWorker.cpp \
           src/TRoutinesDelegate.cpp \
           src/TRoutinesFilterModel.cpp \
           src/TRoutinesModel.cpp \
					 src/TSettings.cpp \
           src/TSettingsDlg.cpp \
           src/TSheduleDelegate.cpp \
           src/TSheduleModel.cpp \
           src/TSpeciesModel.cpp \
           src/TSqlRelation.cpp \
           src/TSqlTableModel.cpp \
           src/TStatusesModel.cpp \
           src/TSubspeciesModel.cpp

UI_DIR = ui_src
DESTDIR = bin
MOC_DIR = moc
OBJECTS_DIR = obj
RCC_DIR = res

win32: RC_ICONS = res/petcore.ico
unix: ICON = ./res/petcore.icns

RESOURCES += res/petcore.qrc
OTHER_FILES +=

#RC_ICONS = res/petcore.ico
TRANSLATIONS = translations/petcore_ru.ts

VERSION = 0.0.7.0
QMAKE_TARGET_COMPANY = Web Success LTD
QMAKE_TARGET_PRODUCT = PetCore
QMAKE_TARGET_DESCRIPTION = PetCore Desktop Tool
QMAKE_TARGET_COPYRIGHT =  Den P.Classen (c)

DEFINES -= VERSION
DEFINES += PETCORE_VERSION=\\\"$$VERSION\\\"

INCLUDEPATH += $$PWD/src/3rdparty/QAesEncryption

# LimeReport
#LR_DIR=$$(lr_deploy_path) # из переменных среды взять
#isEmpty(LR_DIR) {
#  win32 | android: LR_DIR = $$[QT_INSTALL_PREFIX]/3rdparty/LimeReport
#  unix: !android: LR_DIR = $$PWD/../LR/install
#  }

#LIBS += -L$$LR_DIR/lib -llimereport -lQtZint
#INCLUDEPATH += $$LR_DIR/include

message($${VERSION})
system(echo '$${VERSION}'>$${PWD}/petcore_version)

QMAKE_CXXFLAGS += -Wno-unused-variable -Wno-unused-but-set-variable -Wno-unused-parameter -Wno-sign-compare -Wno-misleading-indentation -Wno-template-id-cdtor -Werror# -Wno-attributes
