#include "mainwindow.h"

#include <QApplication>

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
        tsTextStream << QString("%1 Debug - %2").arg(sCurrDateTime).arg(msg)<<Qt::endl;
        break;
    case QtWarningMsg:
        tsTextStream << QString("%1 Warning - %2").arg(sCurrDateTime).arg(msg)<<Qt::endl;
        break;
    case QtCriticalMsg:
        tsTextStream << QString("%1 Critical - %2").arg(sCurrDateTime).arg(msg)<<Qt::endl;
        break;
    case QtInfoMsg:
        tsTextStream << QString("%1 Info - %2").arg(sCurrDateTime).arg(msg)<<Qt::endl;
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
    QApplication a(argc, argv);
    qInstallMessageHandler(myMessageOutput);


    MainWindow w;
    w.show();
    return a.exec();
}
