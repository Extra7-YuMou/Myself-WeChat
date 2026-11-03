#ifndef RESETDIALOG_H
#define RESETDIALOG_H

#include <QDialog>
#include "Global.h"
#include <QJsonObject>

namespace Ui {
class ResetDialog;
}

class ResetDialog : public QDialog
{
    Q_OBJECT
    using Handler = std::function<void(const QJsonObject& json)>;
public:
    explicit ResetDialog(QWidget *parent = nullptr);
    ~ResetDialog();

signals:
    void switchLogin();

private slots:
    void on_get_verify_btn_clicked();

    void on_confirm_btn_clicked();
    void slot_reset_mod_finish(ReqId id, QString res, ErrorCodes err);

    void on_return_btn_clicked();

private:
    bool checkUser();
    bool checkEmail();
    bool checkNewPwd();
    bool checkVerify();
    void addTipErr(TipErr te, QString tip);
    void delTipErr(TipErr te);
    void showTip(QString tip, bool result);
    void initHandlers();

    Ui::ResetDialog *ui;
    QMap<TipErr, QString> _tips;
    QMap<ReqId, Handler> _handlers;
};

#endif // RESETDIALOG_H
