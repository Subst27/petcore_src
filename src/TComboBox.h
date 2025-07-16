#pragma once

#include <QComboBox>

class TSqlTableModel;

class TComboBox : public QComboBox
  {
    Q_OBJECT
  public:
    explicit TComboBox(QWidget *parent=nullptr);
    void showPopup() override;
    void hidePopup() override;
    void setModel(TSqlTableModel *model, const QStringList &valueFields, const QString &indexField, const QString &filter=QString());
    QMap <QString, QVariant> data(qint32 index);
    void setReadOnly(bool readOnly);

  protected:
    void keyReleaseEvent(QKeyEvent *event) override;

  private:
    QMap<qint32, QMap<QString, QVariant>> m_data;
    bool m_readOnly;

  signals:
    void keyPressed();
    void popupShown();
    void popupHidden();

  };
