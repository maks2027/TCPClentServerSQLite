#include "tcpserver.h"
#include <QSqlQuery>
#include <QCoreApplication>

TcpServer::TcpServer(int port, QObject *parent) : QObject(parent)
{
    tcpServer = new QTcpServer(this);

    connect(tcpServer, &QTcpServer::newConnection, this, &TcpServer::newConnection);

    if (tcpServer->listen(QHostAddress::Any, port))
    {
        qInfo() << "TCPSocket "<< QCoreApplication::applicationVersion() << " listen on port " << port;
    }
    else
    {
        qCritical() <<  QObject::tr("Unable to start the server: %1.").arg(tcpServer->errorString());
    }
}

void TcpServer::newConnection()
{
    while (tcpServer->hasPendingConnections())
    {
        QTcpSocket *socket = tcpServer->nextPendingConnection();
        connect(socket, &QTcpSocket::readyRead, this, &TcpServer::readData);
        connect(socket, &QTcpSocket::disconnected, this, &TcpServer::disconnected);

        QByteArray *buffer = new QByteArray();
        QHostAddress adr = socket->peerAddress();

        qInfo() << "New connect" << socket->peerAddress().toString();

        if(socket->socketDescriptor() >= 0)
            socketsList.insert(socket->socketDescriptor(), itemSocket(buffer,socket));
        else
        {
            delete buffer;
            socket->deleteLater();
        }
    }
}

void TcpServer::readData()
{
    QTcpSocket *socket = static_cast<QTcpSocket*>(sender());
    if(socket == nullptr) return;

    QByteArray *buffer = socketsList.value(socket->socketDescriptor()).buffer;
    if(buffer == nullptr) return;

    while (socket->bytesAvailable() > 0)
    {
        buffer->append(socket->readAll());

        if(buffer->size()>1000) buffer->clear();
    }

     pars(socket->socketDescriptor(),buffer);   
}

void TcpServer::pars(int index, QByteArray *buffer)
{
    bool isData = true;
    while(isData)
    {
        int startInd = buffer->indexOf("::");
        int endInd = buffer->indexOf(";;");
        if(endInd > 0 && startInd >= 0 && endInd > startInd)
        {
            QByteArray data = buffer->mid(startInd+2, endInd-2);
            buffer->remove(0, endInd+2);

            emit inData(index,data);
        }
        else
        {
            isData = false;
        }
    }
}

void TcpServer::writeData(int index, QByteArray data)
{
    QTcpSocket *socket = socketsList.value(index).socket;
    if(socket == nullptr) return;

    QByteArray temp;
    temp += "::";
    temp += data;
    temp += ";;";   

    socket->write(temp);
}

void TcpServer::disconnected()
{
    QTcpSocket *socket = static_cast<QTcpSocket*>(sender());
    if(socket == nullptr) return;

    QByteArray *buffer = socketsList.value(socket->socketDescriptor()).buffer;
    if(buffer == nullptr) return;


    socketsList.remove(socket->socketDescriptor());

    qInfo() << "Disconect" <<  socket->peerAddress().toString();

    socket->deleteLater();
    delete buffer;    
}

