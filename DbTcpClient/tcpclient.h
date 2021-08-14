#ifndef TCPCLIENT_H
#define TCPCLIENT_H

#include <QIODevice>
#include <QObject>
#include <QtNetwork>
#include <QTcpSocket>



class TcpClient : public QObject
{
    Q_OBJECT

    struct dataSocket
    {
        dataSocket(){}
        dataSocket(QTimer *timer,QByteArray *inBuffer,QByteArray *outBuffer,QString date1, QString date2,QString host,int port) :
            timer(timer),
            inBuffer(inBuffer),
            outBuffer(outBuffer),
            date1(date1),
            date2(date2),
            host(host),
            port(port){}


        QTimer *timer;
        QByteArray *inBuffer = nullptr;
        QByteArray *outBuffer = nullptr;
        QString date1;
        QString date2;
        QString host;
        int port;
    };

public:
    explicit TcpClient(QObject *parent = nullptr);

    void readValue(QString date1, QString date2, QStringList list, QString host, int port);

signals:
    void inData(QByteArray data,QString date1, QString date2, QString host, int port);
    void noData(QString host, int port);

private:
    QHash<QTcpSocket*, dataSocket> socketsList;
    bool pars(QByteArray *buffer,QString date1, QString date2, QString host, int port);

    QTcpSocket* searchSocket(QTimer * timer);

    void removeSocket(QTcpSocket* socket);
private slots:
    void readData();
    void slotConnected();
    void slotError(QAbstractSocket::SocketError err);
    void slotDisconnected();
    void slotTimeout();
};

#endif // TCPCLIENT_H
