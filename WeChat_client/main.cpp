#include "mainwindow.h"

#include <QApplication>
#include <QFile>
#include <QDir>
#include <QSettings>
#include "Global.h"
#include <QString>
#include "tcpmgr.h"
#include "filetcpmgr.h"

/******************************************************************************
 *
 * @file            main.cpp
 * @brief           XXXX Function
 * @description
 *
 * @author          YuHaoNan
 * @date            2026/04/13
 * @history
 *****************************************************************************/
int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    a.setWindowIcon(QIcon(":/images/WeChat.ico"));
    a.setApplicationName("WeChat");

    QString appPath = QCoreApplication::applicationDirPath();
    QSettings* setting = new QSettings(QSettings::tr("%1/config.ini").arg(appPath), QSettings::IniFormat);
    QString gate_host = setting->value("GateServer/Host").toString();
    QString gate_port = setting->value("GateServer/Port").toString();
    gate_url_prefix = "http://" + gate_host + ":" + gate_port;

    QFile qssFile(":/style/stylesheet.qss");
    if(qssFile.open(QFile::ReadOnly)){
        QString styleSheet = qssFile.readAll();
        a.setStyleSheet(styleSheet);
        qDebug() << "qss 文件打开成功";
        qssFile.close();
    }else{
        qDebug() << "qss 文件打开失败";
    }

    SqlMgr::GetInstance();

    TcpThread thread;
    FileTcpThread file_thread;
    MainWindow w;
    w.setWindowTitle("WeChat");
    w.show();
    return QCoreApplication::exec();
}
