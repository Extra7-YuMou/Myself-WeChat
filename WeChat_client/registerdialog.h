#ifndef REGISTERDIALOG_H
#define REGISTERDIALOG_H

#include <QDialog>
#include "Global.h"
#include <QMap>
#include <QTimer>

namespace Ui {
class RegisterDialog;
}

/******************************************************************************
 *
 * @file            registerdialog.h
 * @brief           注册窗口
 * @description
 *
 * @author          YuHaoNan
 * @date            2026/04/13
 * @history
 *****************************************************************************/
class RegisterDialog : public QDialog
{
    Q_OBJECT

public:
    explicit RegisterDialog(QWidget *parent = nullptr);
    ~RegisterDialog();

signals:
    void switchLogin();
    void switchLogin2();

private slots:
    void on_get_verify_btn_clicked();
    void slot_reg_mod_finish(ReqId id, QString res, ErrorCodes err);

    void on_cancel_btn_clicked();

    void on_confirm_btn_clicked();

    void on_pushButton_clicked();

private:
    void showTip(QString tip, bool result);
    void initHandlers();
    bool checkUser();
    bool checkEmail();
    bool checkPwd();
    bool checkConfirm();
    bool checkVerify();
    void addTipErr(TipErr te, QString tip);
    void delTipErr(TipErr te);
    void changeTipPage();

    Ui::RegisterDialog *ui;
    QMap<ReqId, std::function<void(const QJsonObject&)>> _handlers;
    QMap<TipErr, QString> _tips;
    int _count;
    QTimer* _timer;
};

#endif // REGISTERDIALOG_H
