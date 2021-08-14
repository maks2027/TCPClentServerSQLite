#include "mainwindow.h"
#include "ui_mainwindow.h"


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    connect(&client, &TcpClient::inData, this, &MainWindow::readNet);
    connect(&client, &TcpClient::noData, this, &MainWindow::readNetEr);

    about = new QMessageBox(this);
    about->setWindowTitle("О программе");
    about->setText(QString("Выгрузка данных (tcp клиент) 1.0.5\nАвтор: Киян Максим Викторович\nQt %0").arg(qVersion()));

    ui->dateEdit_1->setDate(QDate::currentDate());
    ui->dateEdit_2->setDate(QDate::currentDate());


    ui->tableWidget->setColumnCount(24 + 2);
    ui->tableWidget->setColumnWidth(0, 75);
    ui->tableWidget->setColumnWidth(1, 50);
    for (int i = 0; i < 24; i++)
    {
        ui->tableWidget->setColumnWidth(i+2, 25);
    }
    ui->tableWidget->horizontalHeader()->hide();

    ui->tableWidget_2->setColumnCount(24 + 2);
    ui->tableWidget_2->setColumnWidth(0, 75);
    ui->tableWidget_2->setColumnWidth(1, 50);
    for (int i = 0; i < 24; i++)
    {
        ui->tableWidget_2->setColumnWidth(i+2, 25);
    }
    ui->tableWidget_2->horizontalHeader()->hide();

    QSettings settings("settings.ini",QSettings::IniFormat);
    settings.beginGroup("MainWindow");
    restoreGeometry(settings.value("geometry", saveGeometry()).toByteArray());
    restoreState(settings.value("state",saveState()).toByteArray());
    settings.endGroup();

    readJsonSettings();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_pushButton_select_clicked()
{
    date1 = ui->dateEdit_1->date();
    date2 = ui->dateEdit_2->date().addDays(1);

    ui->pushButton_select->setEnabled(false);

    queryNet();
}

void MainWindow::createAllTab()
{
    QDate tempDate = date1;

    calcul();

    ui->tableWidget->setRowCount(0);
    ui->tableWidget_2->setRowCount(0);

    int pos1 = 0;
    int pos2 = 0;
    for(int i = 0; tempDate < date2; i++, tempDate = tempDate.addDays(1))
    {
        pos1 = createDayTab(i,pos1, tempDate,ui->tableWidget,false);
        pos2 = createDayTab(i,pos2, tempDate,ui->tableWidget_2,true);
    }

    ui->pushButton_select->setEnabled(true);
    ui->pushButton_select7->setEnabled(true);
}

int MainWindow::createDayTab(int num,int pos, QDate date, QTableWidget *table, bool r)
{
    table->insertRow(pos);

    table->setItem(pos,0, new QTableWidgetItem(date.toString("dd.MM.yyyy")));
    table->item(pos,0)->setBackground(QColor(200,200,200));
    table->setItem(pos,1, new QTableWidgetItem("Сумма"));
    table->item(pos,1)->setBackground(QColor(200,200,200));

    for(int i = 0;i < 24;i++)
    {
        table->setItem(pos,i+2, new QTableWidgetItem(QString("%0:00").arg(i+1)));
        table->item(pos,i+2)->setBackground(QColor(200,200,200));
    }

    pos++;

    int locGroup = -1;
    int groupPos = -1;
    QVector<int> sums(24,0);

    for(int i = 0;i < tabViewList.size();i++)
    {
        if(tabViewList[i].group >= 0 && tabViewList[i].group != locGroup)//создаём группу
        {
            if(groupPos >= 0)//сумируем предедущие
            {
                int sum = 0;
                for(int j = 0; j < 24; j++)
                {
                    table->setItem(groupPos,j+2, new QTableWidgetItem(QString::number(sums[j])));
                    table->item(groupPos,j+2)->setBackground(QColor(240,150,150));
                    sum += sums[j];
                    sums[j] = 0;
                }

                table->setItem(groupPos,1, new QTableWidgetItem(QString::number(sum)));
                table->item(groupPos,1)->setBackground(QColor(240,150,150));
            }


            locGroup = tabViewList[i].group;
            groupPos = pos;

            table->insertRow(pos);

            table->setItem(pos,0, new QTableWidgetItem(getGroupName(tabViewList[i].group)));
            table->item(pos,0)->setBackground(QColor(240,150,150));

            pos++;
        }


        table->insertRow(pos);
        table->setItem(pos,0, new QTableWidgetItem(tabViewList[i].name));

        int sum = 0;

        for(int j = 0; j < 24; j++)
        {
            int index = (num * 24) + j;

            if(r)
            {
                if(index < tabViewList[i].calValues.size())
                {
                    sum += tabViewList[i].calValues[index];
                    sums[j] += tabViewList[i].calValues[index];
                    table->setItem(pos,j+2, new QTableWidgetItem(QString::number(tabViewList[i].calValues[index])));
                }
                else
                {
                    table->setItem(pos,j+2, new QTableWidgetItem("-"));
                }
            }
            else
            {
                if(index < tabViewList[i].values.size())
                {
                    sum += tabViewList[i].values[index];
                    sums[j] += tabViewList[i].values[index];
                    table->setItem(pos,j+2, new QTableWidgetItem(QString::number(tabViewList[i].values[index])));
                }
                else
                {
                    table->setItem(pos,j+2, new QTableWidgetItem("-"));
                }
            }
        }
        table->setItem(pos,1, new QTableWidgetItem(QString::number(sum)));

        pos++;
    }

    if(groupPos >= 0)//сумируем предедущие
    {
        int sum = 0;
        for(int j = 0; j < 24; j++)
        {
            table->setItem(groupPos,j+2, new QTableWidgetItem(QString::number(sums[j])));
            table->item(groupPos,j+2)->setBackground(QColor(240,150,150));
            sum += sums[j];
            sums[j] = 0;
        }

        table->setItem(groupPos,1, new QTableWidgetItem(QString::number(sum)));
        table->item(groupPos,1)->setBackground(QColor(240,150,150));
    }

    return pos;
}

void MainWindow::readJsonSettings()
{
    QString path = QDir::currentPath() + "/servers.txt";

    if(!QFile::exists(path))
    {
        qCritical()<< "no json file";

        QMetaObject::invokeMethod(qApp, "quit", Qt::QueuedConnection);
        //qApp->quit();
        return;
    }

    QString val;
    QFile file;
    file.setFileName(path);
    file.open(QIODevice::ReadOnly | QIODevice::Text);
    val = file.readAll();
    file.close();

    QJsonDocument doc = QJsonDocument::fromJson(val.toUtf8());
    QJsonObject json = doc.object();

    QJsonArray servers = json["Servers"].toArray();

    for(int i = 0; i < servers.size();i++)
    {
        QJsonObject server = servers[i].toObject();

        stServer tempServer;

        tempServer.ip = server["Ip"].toString();
        tempServer.port = server["Port"].toInt();

        QJsonArray tabs = server["Tabs"].toArray();
        for(int j = 0; j < tabs.size();j++)
        {
            QJsonObject tab = tabs[j].toObject();

            tabView tempTabWiew;
            tempTabWiew.index = tab["Index"].toInt();
            tempTabWiew.group = tab["Group"].toInt();
            tempTabWiew.name = tab["Name"].toString();
            tempTabWiew.tabName = tab["Tab"].toString();
            tabViewList.append(tempTabWiew);

            tempServer.tabs << tab["Tab"].toString();
        }

        severlist.append(tempServer);
    }

    if(severlist.isEmpty())
    {
        qCritical()<< "Servers is empty";

        QMetaObject::invokeMethod(qApp, "quit", Qt::QueuedConnection);
    }

    std::sort(tabViewList.begin(), tabViewList.end(), [](tabView& a, tabView& b) { return std::tie(a.group, a.index) < std::tie(b.group, b.index); } );

    QJsonArray tavCal = json["Calcul"].toArray();

    for(int i = 0; i < tavCal.size();i++)
    {
        calTab tab;

        QJsonObject objTab = tavCal[i].toObject();

        tab.tab1 = objTab["TabEq"].toString();
        tab.tab2 = objTab["TabD"].toString();

        calTabList.append(tab);
    }

    QJsonArray groupNames = json["GroupNames"].toArray();

    for(int i = 0; i < groupNames.size();i++)
    {
        Group groupTemp;

        QJsonObject objGroup = groupNames[i].toObject();

        groupTemp.index = objGroup["Index"].toInt();
        groupTemp.name =  objGroup["Name"].toString();

        groupsList.append(groupTemp);
    }

    bool only7day = json["Only7day"].toBool();
    if(only7day)
    {
        ui->pushButton_select->setHidden(true);
        ui->dateEdit_1->setHidden(true);
        ui->dateEdit_2->setHidden(true);

    }

    bool LaunchSelect = json["LaunchSelect"].toBool();
    if(LaunchSelect)
    {
        on_pushButton_select7_clicked();
    }

}

void MainWindow::readJsonNet(QByteArray data)
{   
    QJsonParseError error;
    QJsonDocument json;
    json = QJsonDocument::fromJson(data, &error);



    if(json.isNull())
    {
        qWarning()<< "Net json err: " << error.errorString();
        return;
    }

    QJsonObject root = json.object();

    QStringList keys = root.keys();


    for(int i = 0; i < keys.size(); i++)
    {
        int tabViewIndex = searchTabView(keys[i]);
        if(tabViewIndex < 0) continue;

        QJsonArray arTab = root.value(keys[i]).toArray();
        if(arTab.size() % 24 != 0 || arTab.isEmpty())continue;

        for (int j = 0;j < arTab.size(); j++)
        {
            tabViewList[tabViewIndex].values.append(arTab.at(j).toInt());
        }
    }
}

int MainWindow::searchServer(QString host, int port)
{
    for(int i = 0; i < severlist.size(); i++)
    {
        if(severlist[i].ip == host && severlist[i].port == port)
            return i;
    }
    return -1;
}

int MainWindow::searchTabView(QString tabName)
{
    for (int i = 0; i < tabViewList.size();i++)
    {
        if(tabViewList[i].tabName == tabName)
            return i;
    }
    return -1;
}

bool MainWindow::isAllRet()
{
    for(int i = 0; i < severlist.size(); i++)
    {
        if(!severlist[i].isRet)
            return false;
    }

    return true;
}

void MainWindow::queryNet()
{
    for(int i = 0; i < severlist.size(); i++)
    {
        severlist[i].isRet = false;

        client.readValue(date1.toString("yyyy-MM-dd"),
                         date2.toString("yyyy-MM-dd"),
                         severlist[i].tabs,
                         severlist[i].ip,
                         severlist[i].port);
    }

    for(int i = 0; i < tabViewList.size(); i++)
    {
        tabViewList[i].values.clear();
        tabViewList[i].calValues.clear();
    }
}

QString MainWindow::getGroupName(int index)
{
    for (int i = 0;i < groupsList.size(); i++)
    {
        if(groupsList[i].index == index)
            return groupsList[i].name;
    }

    return QString();
}

void MainWindow::readNet(QByteArray data, QString date1, QString date2, QString host, int port)
{    
    Q_UNUSED(date1);
    Q_UNUSED(date2);
    readJsonNet(data);
    int p = searchServer(host, port);

    if(p < 0) return;

    severlist[p].isRet = true;

    if(isAllRet())
    {
        createAllTab();
    }
}

void MainWindow::readNetEr(QString host, int port)
{
    qWarning()<< "No data: " << host << ":" << port;
    int p = searchServer(host, port);

    if(p < 0) return;

    severlist[p].isRet = true;

    if(isAllRet())
    {
        createAllTab();
    }
}

void MainWindow::calcul()
{
    for(int i = 0; i < calTabList.size(); i++)
    {
        int x2 = searchTabView(calTabList[i].tab1);
        int x1 = searchTabView(calTabList[i].tab2);

        if(x1 < 0 || x1 < 0) continue;

        if(tabViewList[x1].values.size() != tabViewList[x2].values.size()) continue;

        for(int j = 0; j < tabViewList[x1].values.size(); j++)
        {
            tabViewList[x2].calValues.append(tabViewList[x2].values[j] - tabViewList[x1].values[j]);
        }
    }

    for(int i = 0;i < tabViewList.size();i++)
    {
        if(tabViewList[i].calValues.isEmpty())
            tabViewList[i].calValues.append(tabViewList[i].values);
    }
}

void MainWindow::on_tableWidget_itemSelectionChanged()
{
    QList<QTableWidgetItem *> tempList = ui->tableWidget->selectedItems();
    int sum = 0;

    for (int i = 0; i < tempList.size();i++)
    {
        QTableWidgetItem *item = tempList[i];

        sum += item->text().toInt(); // если не число то 0
    }

    ui->label_sum1->setText(QString("Сумма: %0").arg(sum));
}

void MainWindow::on_tableWidget_2_itemSelectionChanged()
{
    QList<QTableWidgetItem *> tempList = ui->tableWidget_2->selectedItems();
    int sum = 0;

    for (int i = 0; i < tempList.size();i++)
    {
        QTableWidgetItem *item = tempList[i];

        sum += item->text().toInt(); // если не число то 0
    }

    ui->label_sum2->setText(QString("Сумма: %0").arg(sum));
}

void MainWindow::on_action_triggered()
{
    about->open();
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    QSettings settings("settings.ini",QSettings::IniFormat);
    settings.beginGroup("MainWindow");

    settings.setValue("geometry", saveGeometry());
    settings.setValue("state", saveState());
    settings.endGroup();

    settings.sync();

    event->accept();
}

void MainWindow::on_pushButton_select7_clicked()
{
    date1 = QDate::currentDate().addDays(-6);
    date2 = QDate::currentDate().addDays(1);

    ui->pushButton_select->setEnabled(false);
    ui->pushButton_select7->setEnabled(false);

    queryNet();
}
