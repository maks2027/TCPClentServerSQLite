#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTableWidget>
#include <QMessageBox>
#include <QCloseEvent>
#include "tcpclient.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

    struct stServer
    {
        QString ip;
        int port;
        QStringList tabs;

        bool isRet = false;
    };

    struct tabView
    {
        int index = -1;
        int group = -1;
        QString name;
        QString tabName;
        QVector<int> values;
        QVector<int> calValues;
    };

    struct calTab
    {
        QString tab1;
        QString tab2;
    };

    struct Group
    {
        int index;
        QString name;
    };

    TcpClient client;

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    QVector<stServer> severlist;
    QVector<tabView> tabViewList;
    QVector<calTab> calTabList;
    QVector<Group> groupsList;

private slots:
    void on_pushButton_select_clicked();

    void on_tableWidget_itemSelectionChanged();

    void on_tableWidget_2_itemSelectionChanged();

    void on_action_triggered();

    void on_pushButton_select7_clicked();

public slots:
    //void writeTab(QByteArray data,QString date1,QString date2);
    void readNet(QByteArray data,QString date1, QString date2, QString host, int port);
    void readNetEr(QString host, int port);

private:
    Ui::MainWindow *ui;
    QMessageBox* about;

    QDate date1;
    QDate date2;

    void calcul();

    void createAllTab();
    int createDayTab(int num, int pos, QDate date, QTableWidget *table, bool r);

    void readJsonSettings();
    void readJsonNet(QByteArray data);

    int searchServer(QString host, int port);
    int searchTabView(QString tabName);
    bool isAllRet();
    void queryNet();

    QString getGroupName(int index);

protected:
    void closeEvent(QCloseEvent *event);

};
#endif // MAINWINDOW_H
