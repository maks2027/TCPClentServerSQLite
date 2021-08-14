#include "tcpclient.h"

TcpClient::TcpClient(QObject *parent) : QObject(parent)
{
   //qRegisterMetaType<QAbstractSocket::SocketError>("QAbstractSocket::SocketError") ;
}

void TcpClient::readValue(QString date1, QString date2, QStringList list, QString host, int port)
{
    QJsonDocument documentObject;
    QJsonObject recordObject;

    recordObject.insert("Date1", date1);
    recordObject.insert("Date2", date2);

    QJsonArray arr;
    for(int i = 0;i < list.size();i++)
    {
        arr.push_back(QJsonValue(list.value(i)));
    }

    recordObject.insert("Tables", arr);
    documentObject.setObject(recordObject);

    QTcpSocket *TcpSocket = new QTcpSocket(this);
    QByteArray *inBuffer = new QByteArray;
    QByteArray *outBuffer = new QByteArray;
    QTimer *timer = new QTimer(this);
    timer->setSingleShot(true);
    timer->start(5000);

    outBuffer->append(documentObject.toJson(QJsonDocument::Compact));

    TcpSocket->connectToHost(host, port);
    connect(TcpSocket, &QTcpSocket::connected, this, &TcpClient::slotConnected);
    connect(TcpSocket, &QTcpSocket::readyRead, this, &TcpClient::readData);
    connect(TcpSocket, &QTcpSocket::disconnected, this, &TcpClient::slotDisconnected);
    connect(timer, &QTimer::timeout, this, &TcpClient::slotTimeout);
    connect(TcpSocket, &QTcpSocket::errorOccurred, this, &TcpClient::slotError);

    socketsList.insert(TcpSocket,dataSocket(timer, inBuffer, outBuffer, date1, date2,host,port));
}

void TcpClient::readData()
{
    QTcpSocket *socket = static_cast<QTcpSocket*>(sender());
    QByteArray *buffer = socketsList.value(socket).inBuffer;
    QTimer *timer = socketsList.value(socket).timer;
    QString date1 = socketsList.value(socket).date1;
    QString date2 = socketsList.value(socket).date2;
    QString host = socketsList.value(socket).host;
    int port = socketsList.value(socket).port;

    if(socket == nullptr || buffer == nullptr ||timer == nullptr) return;

    while(socket->bytesAvailable() > 0)
    {
        buffer->append(socket->readAll());

        if(buffer->size()>10000)
        {
            buffer->clear();
            qCritical()<<"Buffer overflow";

        }
    }

    if(pars(buffer,date1,date2,host,port))
        socket->disconnectFromHost();
}

void TcpClient::slotConnected()
{
    QTcpSocket *socket = static_cast<QTcpSocket*>(sender());
    QByteArray *buffer = socketsList.value(socket).outBuffer;
    QTimer *timer = socketsList.value(socket).timer;

    if(socket == nullptr || timer == nullptr) return;

    timer->start(10000);

    QByteArray temp;
    temp += "::";
    temp += *buffer;
    temp += ";;";

    socket->write(temp);
}

void TcpClient::slotError(QAbstractSocket::SocketError err)
{
    //if(err==0) return;
     Q_UNUSED(err);

    QTcpSocket *socket = static_cast<QTcpSocket*>(sender());
    if(socket == nullptr) return;

    qWarning()<< socket->errorString();

    QString host = socketsList.value(socket).host;
    int port = socketsList.value(socket).port;

    emit noData(host, port);

    removeSocket(socket);
}

void TcpClient::slotDisconnected()
{
    QTcpSocket *socket = static_cast<QTcpSocket*>(sender());
    removeSocket(socket);
}

void TcpClient::slotTimeout()
{    
    QTimer *timer = static_cast<QTimer*>(sender());

    QTcpSocket *socket = searchSocket(timer);
    if(socket == nullptr) return;

    QString host = socketsList.value(socket).host;
    int port = socketsList.value(socket).port;

    emit noData(host, port);
    socket->disconnectFromHost();

    //removeSocket(socket);
}

bool TcpClient::pars(QByteArray *buffer, QString date1, QString date2, QString host, int port)
{
    //    bool isData = true;
    //    while(isData)
    //    {
    int startInd = buffer->indexOf("::");
    int endInd = buffer->indexOf(";;");
    if(endInd > 0 && startInd >= 0 && endInd > startInd)
    {
        QByteArray data = buffer->mid(startInd+2, endInd-2);
        buffer->remove(0, endInd+2);

        emit inData(data, date1, date2, host, port);

        return true;
    }
    //        else
    //        {
    //            isData = false;
    //        }
    //    }

    return false;
}

QTcpSocket *TcpClient::searchSocket(QTimer *timer)
{
    foreach (QTcpSocket *socket, socketsList.keys())
    {
        if(socketsList.value(socket).timer == timer)
            return socket;
    }

    return nullptr;
}

void TcpClient::removeSocket(QTcpSocket *socket)
{
    if(socket == nullptr) return;
    QTimer *timer = socketsList.value(socket).timer;

    QByteArray *inBuffer = socketsList.value(socket).inBuffer;
    QByteArray *outBuffer = socketsList.value(socket).outBuffer;

    socket->deleteLater();
    timer->deleteLater();

    delete inBuffer;
    delete outBuffer;

    socketsList.remove(socket);
}
