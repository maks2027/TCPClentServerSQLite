#include "dataread.h"
#include <QJsonDocument>
#include <QJsonObject>

DataRead::DataRead(QObject *parent) : QObject(parent)
{
    
}

void DataRead::setBdPath(QString path)
{
    if(!QFile(path).exists())
    {
        qCritical()<<"No file db";
    }

    dbPath = path;
}

void DataRead::command(int index, QByteArray data)
{    
    QJsonParseError error;
    QJsonDocument json;
    json = QJsonDocument::fromJson(data, &error);
    
    if(json.isNull())
    {
        qCritical()<<"Error json: " << error.errorString();
        return;
    }
    
    QJsonObject root = json.object();
    
    QString date1 = root.value("Date1").toString();
    QString date2 = root.value("Date2").toString();
    QStringList list;
    
    QJsonValue jv = root.value("Tables");
    if(jv.isArray())
    {
        QJsonArray ja = jv.toArray();
        for(int i = 0; i < ja.count(); i++)
        {
            list << ja.at(i).toString();
        }
    }

    works(date1, date2, list, index);
}

bool DataRead::createMemDb(QString &date1, QString &date2, QStringList &list)
{
    if(QSqlDatabase::contains("mem"))
    {
        QSqlDatabase::removeDatabase("mem");
    }

    {
        QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE","mem");
        db.setDatabaseName("file::memory:?cache=shared");
        db.setConnectOptions("QSQLITE_ENABLE_SHARED_CACHE");
        db.setConnectOptions("QSQLITE_OPEN_URI");
        if (!db.open())
        {
            qCritical() << "Memdb not open";
            return false;
        }
    }
    
    QSqlDatabase db = QSqlDatabase::database("mem");
    QSqlQuery query(db);
    QString sql;
    
    for(int i = 0;i < list.size();i++)
    {
        QString sql = QString("CREATE TABLE `%0` (`id` INTEGER, `value` INTEGER, `date` TEXT, PRIMARY KEY(`id`))").arg(list[i]);
        if (!query.exec(sql))
        {
            qCritical() << "Error CREATE";
            db.close();
            return false;
        }
    }
    
    if(!QFile(dbPath).exists())
    {
        qCritical()<<"No file db";
        db.close();
        return false;
    }

    sql = QString("ATTACH DATABASE '%0' AS file;").arg(dbPath);
    if (!query.exec(sql))
    {
        qCritical() << "Memdb not ATTACH file";
        db.close();
        return false;
    }
    
    for(int i = 0;i < list.size();i++)
    {
        QString sql = QString("INSERT INTO %0 SELECT * FROM file.%0 WHERE (`date` BETWEEN \"%1\" AND \"%2\");").arg(list[i]).arg(date1).arg(date2);

        if (!query.exec(sql))
        {
            qCritical() << "Error INSERT INTO";
            db.close();
            return false;
        }
    }

    sql = QString("DETACH DATABASE 'file';");
    if (!query.exec(sql))
    {
        qCritical() << "Error DETACH";
        db.close();
        return false;
    }

    return true;
}

void DataRead::works(QString &date1, QString &date2, QStringList &list, int index)
{    
    qInfo()<< "Start read DB";
    if(createMemDb(date1, date2, list))
    {
        dbRead(date1, date2, list, index);
    }
    qInfo()<< "Stop read DB";
}

void DataRead::dbRead(QString &date1, QString &date2, QStringList &list, int index)
{
    QSqlDatabase db = QSqlDatabase::database("mem");
    QSqlQuery query(db);
    
    if(!db.transaction())
    {
        emit sendError(index,"No transaction mem");
        qCritical()<<"No transaction mem";
        return;
    }
    
    QDateTime startTime;
    startTime.setDate(QDate::fromString(date1,"yyyy-MM-dd"));
    QDateTime endTime;
    endTime.setDate(QDate::fromString(date2,"yyyy-MM-dd"));

    QJsonDocument documentObject;
    QJsonObject recordObject;

    for(int i = 0;i < list.size();i++)
    {
        QDateTime hour = startTime;
        QJsonArray arr;
        while(hour < endTime)
        {
            QString time1 = hour.toString("yyyy-MM-dd hh");
            hour = hour.addSecs(3600);
            QString time2 = hour.toString("yyyy-MM-dd hh");
            
            QString sql;
            sql = QString("SELECT sum(value) FROM `%0` WHERE (`date` BETWEEN \"%1\" AND \"%2\")").arg(list.at(i)).arg(time1).arg(time2);
            if (!query.exec(sql))
            {
                qCritical() << "error SELECT in " << list.at(i);
            }

            while(query.next())//должны получить 24 * количество_дней значений
            {
                arr.push_back(QJsonValue(query.value(0).toInt()));
            }

        }
        recordObject.insert(list.at(i), arr);
    }
    
    db.commit();
    db.close();

    documentObject.setObject(recordObject);

    emit sendData(index, documentObject.toJson(QJsonDocument::Compact));
}
