#ifndef TCPSERVER_H
#define TCPSERVER_H

#include <QObject>
#include <QtNetwork>
#include <QTcpSocket>

class TcpServer : public QObject
{
    Q_OBJECT

    struct itemSocket
    {
        itemSocket(){}
        itemSocket(QByteArray *buffer, QTcpSocket *socket) : buffer(buffer), socket(socket) {}

        QByteArray *buffer = nullptr;
        QTcpSocket *socket = nullptr;
    };

public:
    explicit TcpServer(int port, QObject *parent = nullptr);

signals:
    void inData(int index, QByteArray data);

private:
    QTcpServer *tcpServer;// = new QTcpServer(this);
    QHash<int, itemSocket> socketsList;

    void pars(int index, QByteArray *buffer);

    void newConnection();
    void readData();
    void disconnected();

public slots:
    void writeData(int index,QByteArray data);

//private slots:

};

#endif // TCPSERVER_H
