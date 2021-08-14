#include <QCoreApplication>
#include <QThread>
#include <iostream>
#include "tcpserver.h"
#include "dataread.h"

void myMessageOutput(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    Q_UNUSED(context);

    QFile fMessFile(qApp->applicationDirPath() + "/log.txt");
    if(!fMessFile.open(QIODevice::Append | QIODevice::Text))
    {
        return;
    }
    QString sCurrDateTime = "[" +
            QDateTime::currentDateTime().toString("dd.MM.yyyy hh:mm:ss.zzz") + "]";
    QTextStream tsTextStream(&fMessFile);
    switch(type){
    case QtDebugMsg:
        //tsTextStream << QString("%1 Debug - %2").arg(sCurrDateTime).arg(msg)<<Qt::endl;
        std::cout << QString("%1 %2").arg(sCurrDateTime).arg(msg).toStdString() << std::endl;
        break;
    case QtWarningMsg:
        tsTextStream << QString("%1 Warning - %2").arg(sCurrDateTime).arg(msg)<<Qt::endl;
        std::cout << QString("%1 %2").arg(sCurrDateTime).arg(msg).toStdString() << std::endl;
        break;
    case QtCriticalMsg:
        tsTextStream << QString("%1 Critical - %2").arg(sCurrDateTime).arg(msg)<<Qt::endl;
        std::cout << QString("%1 %2").arg(sCurrDateTime).arg(msg).toStdString() << std::endl;
        break;
    case QtInfoMsg:
        //tsTextStream << QString("%1 Info - %2").arg(sCurrDateTime).arg(msg)<<Qt::endl;
        std::cout << QString("%1 %2").arg(sCurrDateTime).arg(msg).toStdString() << std::endl;
        break;
    case QtFatalMsg:
        tsTextStream << QString("%1 Fatal - %2").arg(sCurrDateTime).arg(msg)<<Qt::endl;        
        abort();
    }
    tsTextStream.flush();
    fMessFile.flush();
    fMessFile.close();
}

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    qInstallMessageHandler(myMessageOutput);

    QString path = QDir::currentPath() + "/settings.txt";

    if(!QFile::exists(path))
    {
        qCritical()<< "No json file";

        QMetaObject::invokeMethod(qApp, "quit", Qt::QueuedConnection);

         return a.exec();
    }

    QString val;
    QFile file;
    file.setFileName(path);
    file.open(QIODevice::ReadOnly | QIODevice::Text);
    val = file.readAll();
    file.close();

    QJsonDocument doc = QJsonDocument::fromJson(val.toUtf8());
    QJsonObject json = doc.object();

    QString pathDb = json["Path"].toString();
    int port = json["Port"].toInt();

    if(pathDb.isEmpty())
    {
        qCritical()<< "No path Db";
        QMetaObject::invokeMethod(qApp, "quit", Qt::QueuedConnection);
        return a.exec();
    }

    if(port == 0)
    {
        qCritical()<< "No port";
        QMetaObject::invokeMethod(qApp, "quit", Qt::QueuedConnection);
        return a.exec();
    }

    TcpServer server(port);

    QThread thread;
    DataRead readData;
    readData.setBdPath(pathDb);

    QObject::connect(&thread, &QThread::finished, &readData, &DataRead::deleteLater);
    QObject::connect(&thread, &QThread::finished, &thread, &QThread::deleteLater);

    readData.moveToThread(&thread);
    thread.start();

    QObject::connect(&server, &TcpServer::inData, &readData, &DataRead::command);
    QObject::connect(&readData, &DataRead::sendData, &server, &TcpServer::writeData);

    return a.exec();
}
