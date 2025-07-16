#include "TSheduleDelegate.h"

#include "TSheduleModel.h"
#include "TApptsModel.h"
#include "TRoutinesModel.h"

#include "TPalette.h"

#include <QSqlRecord>
#include <QApplication>
#include <QDebug>

TSheduleDelegate::TSheduleDelegate(QObject *parent) : QStyledItemDelegate{parent}
  {
  /* суббота       - TPalette::Saurday
   * воскресенье   - TPalette::Sunday
   * недоступный   - TPalette::Unavailable
   * протухший     - TPalette::Missed
   * начавшийся    - TPalette::Started
   * закончившийся - TPalette::Finished
   * ждем клиента  - TPalette::Waiting */
  }

void TSheduleDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
  {
  const TSheduleModel *sheduleModel=qobject_cast<const TSheduleModel*>(index.model());
  if (sheduleModel==nullptr || index.isValid()==false)
    {
    QStyledItemDelegate::paint(painter,option,index);
    return;
    }

  QStyleOptionViewItem itemOption(option);

  // date и time оставил в TSheduleModel самостоятельными специально для удобства, можно тащить напрямую
  QDate date=sheduleModel->index(index.row(),TSheduleModel::Date,index.parent()).data().toDate();
  QTime time=sheduleModel->index(index.row(),TSheduleModel::Time,index.parent()).data().toTime();
  // разница между назначенным и текущим временем, может быть отрицательной
  qint32 delta=QDateTime::currentDateTime().secsTo(QDateTime(date,time));
  TSheduleModel::Types type=(TSheduleModel::Types)(sheduleModel->index(index.row(),TSheduleModel::Type,index.parent()).data().toUInt());

  QHash<QString,QVariant> apptHash;
  QHash<QString,QVariant> routineHash;

  QString theme=qApp->property("app_theme").toString();
  // цвет текста в зависимости от того, запись уже в прошлом или еще нет
  itemOption.palette.setColor(QPalette::Text, delta<0 ? TPalette::color(TPalette::Unavailable,TPalette::Foregroud) : qApp->palette().windowText().color());
  // вариации на тему цвета фона в заивисомсти от разных ситуаций и типов ноды
  switch (type)
    {
    case TSheduleModel::DateType:
      {
      if (date.dayOfWeek()==6) // суббота
        {
        itemOption.backgroundBrush=TPalette::color(TPalette::Saturday,TPalette::Background);
        itemOption.palette.setColor(QPalette::Text,TPalette::color(TPalette::Saturday,TPalette::Foregroud));
        }

      if (date.dayOfWeek()==7) // воскресенье
        {
        itemOption.backgroundBrush=TPalette::color(TPalette::Sunday,TPalette::Background);
        itemOption.palette.setColor(QPalette::Text,TPalette::color(TPalette::Sunday,TPalette::Foregroud));
        }

      break;
      }
    case TSheduleModel::DoctorType:
      {
      // в DataRecord лежат данные из TRoutinesModel
      routineHash=sheduleModel->index(index.row(),TSheduleModel::DataRecord,index.parent()).data().toHash();
      if (routineHash.value("available").toBool()==false)
        {
        itemOption.backgroundBrush=TPalette::color(TPalette::Unavailable,TPalette::Background);
        itemOption.palette.setColor(QPalette::Text,TPalette::color(TPalette::Unavailable,TPalette::Foregroud));
        }

      break;
      }
    case TSheduleModel::TimeType:
      {
      // в DataRecord лежат данные из TApptsModel, в DataRecord родителя лежат данные из TRoutinesModel
      apptHash=sheduleModel->index(index.row(),TSheduleModel::DataRecord,index.parent()).data().toHash();
      routineHash=sheduleModel->index(index.parent().row(),TSheduleModel::DataRecord,index.parent().parent()).data().toHash();

      TApptsModel::States state=(TApptsModel::States)apptHash.value("state").toUInt();
      if (state==TApptsModel::Started) // начвшийся
        {
        itemOption.backgroundBrush=TPalette::color(TPalette::Started,TPalette::Background);
        itemOption.palette.setColor(QPalette::Text,TPalette::color(TPalette::Started,TPalette::Foregroud));
        break;
        }

      if (state==TApptsModel::Finished) // завершенный
        {
        itemOption.backgroundBrush=TPalette::color(TPalette::Finished,TPalette::Background);
        itemOption.palette.setColor(QPalette::Text,TPalette::color(TPalette::Finished,TPalette::Foregroud));
        break;
        }

      // остается Created
      quint32 apptId=apptHash.value("id").toUInt();
      quint16 interval=routineHash.value("interval").toUInt();

      if (routineHash.value("available").toBool()==false)
        {
        // если "недоступен", но есть запись на прием на это время, то пометить ее "протухшей", если нету - просто время недоступно
        itemOption.backgroundBrush=apptId>0 ? TPalette::color(TPalette::Missed,TPalette::Background) : TPalette::color(TPalette::Unavailable,TPalette::Background);
        itemOption.palette.setColor(QPalette::Text,TPalette::color(TPalette::Unavailable,TPalette::Foregroud));
        break;
        }

      // если не стартовали, не финишировали, запись на прием есть, ее время прошло, но еще не критически
      if (apptId>0 && delta<0 && qAbs(delta)<30*interval)
        {
        itemOption.backgroundBrush=TPalette::color(TPalette::Waiting,TPalette::Background);
        itemOption.palette.setColor(QPalette::Text,TPalette::color(TPalette::Waiting,TPalette::Foregroud));
        }

       break;
      }
    }

  // если нет реальной инфы в индексе - то уходим
  if (index.data().isValid()==false)
    {
    qApp->style()->drawControl(QStyle::CE_ItemViewItem,&itemOption,painter);
    return;
    }

  // отображение текста, всю инфу сводим в столбец "Title" по определенным правилам
  itemOption.text=index.data().toString();
  if ((TSheduleModel::Columns)index.column()==TSheduleModel::Title)
    {
    itemOption.features.setFlag(QStyleOptionViewItem::HasDecoration,true);
    itemOption.decorationSize=QSize(16,16);
    itemOption.decorationPosition=QStyleOptionViewItem::Left;

    switch (type)
      {
      case TSheduleModel::DateType:
        {
        QStringList weekDays={tr("Monday"),tr("Tuesday"),tr("Wednesday"),tr("Thursday"),tr("Friday"),tr("Saturday"),tr("Sunday")};

        itemOption.text=QString("%1 (%2)").arg(date.toString("dd.MM.yyyy"),weekDays.at(date.dayOfWeek()-1));
        itemOption.icon=QIcon(QString(":/theme_icons/images/%1/appts").arg(theme));
        break;
        }
      case TSheduleModel::DoctorType:
        {
        QString doctorName=routineHash.value("name").toString();
        QString profile=routineHash.value("profile").toString();
        QString phoneNumber=routineHash.value("phone_number").toString();
        QString telegram=routineHash.value("telegram").toString();

        itemOption.text=tr("%1 (%2), phone: %3").arg(doctorName,profile,phoneNumber);
        if (telegram.isEmpty()==false)
          itemOption.text.append(tr(", telegram id: %1").arg(telegram));

        itemOption.icon=QIcon(QString(":/theme_icons/images/%1/doctors").arg(theme));
        break;
        }
      case TSheduleModel::TimeType:
        {
        itemOption.text=time.toString("hh:mm");
        if (apptHash.value("id").toUInt()<1)
          {
          itemOption.icon=delta<0 ? QIcon(QString(":/theme_icons/images/%1/clock_na").arg(theme)) : QIcon(QString(":/theme_icons/images/%1/clock").arg(theme));
          break;
          }

        // дальше добавить подробности про назначенный прием
        QString action=apptHash.value("action").toString();
        QString state=TApptsModel::textByState((TApptsModel::States)apptHash.value("state").toUInt());

        itemOption.text.append(QString(" - %2").arg(action));
        if (state.isEmpty()==false)
          itemOption.text.append(QString(" (%1)").arg(state));

        QString clientName=apptHash.value("client_name").toString();
        QString phoneNumber=apptHash.value("phone_number").toString();
        QString telegram=apptHash.value("telegram").toString();

        itemOption.text.append(tr("; %1, phone: %2").arg(clientName,phoneNumber));
        if (telegram.isEmpty()==false)
          itemOption.text.append(tr(", telegram id: %1").arg(telegram));

        QString petName=apptHash.value("pet_name").toString();
        QString species=apptHash.value("species").toString();
        QString breed=apptHash.value("breed").toString();

        itemOption.text.append(QString("; %1, %2").arg(petName,species));
        if (breed.isEmpty()==false)
          itemOption.text.append(QString(", %1").arg(breed));

        itemOption.icon=QIcon(QString(":/theme_icons/images/%1/clock_na").arg(theme));
        break;
        }
      }
    }

  qApp->style()->drawControl(QStyle::CE_ItemViewItem,&itemOption,painter);
  }

QSize TSheduleDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
  {
  Q_UNUSED(index)
  return QSize(option.rect.width(),qApp->font().pointSize()*2.5);
  }
