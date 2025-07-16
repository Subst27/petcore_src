#pragma once

#include "ui_TDoctorDlg.h"
#include "TModelDlg.h"

class TRoutinesModel;
class TRoutinesFilterModel;

class TDoctorDlg : public TModelDlg, private Ui::TDoctorDlg
  {
  Q_OBJECT

  public:
    explicit TDoctorDlg(const QSqlRecord &record, QWidget *parent = nullptr);
    QSqlRecord record() const;

  protected:
    void writeSettings() override;
    void readSettings() override;

    void checkShedule();

  protected slots:
    void phonePostionChenged(int oldPos,int newPos);
    void selectPreffered();
    void copyRoutine();

    void accept();
    void reject();

    void showSuggest();

  private:
    QString m_lastText;
    QTimer *m_suggestTimer;

    TRoutinesModel *m_routinesModel;
    TRoutinesFilterModel *m_oddRoutineModel;
    TRoutinesFilterModel *m_evenRoutineModel;

    bool m_availableChanged; // флаг, что поменялась доступность для записи доктора
    bool m_intervalChanged;  // флаг, что поменялся интервал приема доктора
    bool m_routinesChanged;  // флаг, что поменялось личное расписание доктора

  signals:
    void routinesChanged();
    void showApptsNeeded(QMap <QDate, QList<quint32>> expanded);
  };
