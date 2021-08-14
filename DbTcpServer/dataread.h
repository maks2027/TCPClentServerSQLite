#ifndef DATAREAD_H
#define DATAREAD_H

#include <QObject>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QtSql>
#include <QDate>
#include <QDateTime>

class DataRead : public QObject
{
    Q_OBJECT

public:
    explicit DataRead(QObject *parent = nullptr);

signals:
    void sendError(int index, QString msg);
    void sendData(int index, QByteArray data);

public slots:
    void setBdPath(QString path);
    void command(int index,QByteArray data);


private:
    QString dbPath;

    void works(QString &date1, QString &date2, QStringList &list, int index);
    void dbRead(QString &date1, QString &date2, QStringList &list, int index);
    bool createMemDb(QString &date1, QString &date2, QStringList &list);
};

#endif // DATAREAD_H
