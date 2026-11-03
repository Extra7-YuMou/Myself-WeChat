#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "LoginDialog.h"
#include "registerdialog.h"
#include "resetdialog.h"
#include "ChatDialog.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

/******************************************************************************
 *
 * @file            mainwindow.h
 * @brief           主窗口
 * @description
 *
 * @author          YuHaoNan
 * @date            2026/04/13
 * @history
 *****************************************************************************/
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

private slots:
    void slotSwitchRegister();
    void slotSwitchLogin();
    void slotSwitchLogin2();
    void slotSwitchReset();
    void slotSwitchLogin3();
    void slotSwitchChat(std::vector<std::shared_ptr<ApplyInfo>> apply_list, QMap<int, std::shared_ptr<UserInfo>>,
                        qint64, QMap<qint64, int>);
    void slotSwitchLogin4(int);

private:
    Ui::MainWindow *ui;
    LoginDialog* _login_dlg;
    RegisterDialog* _reg_dlg;
    ResetDialog* _reset_dlg;
    ChatDialog* _chat_dlg;
};
#endif // MAINWINDOW_H
