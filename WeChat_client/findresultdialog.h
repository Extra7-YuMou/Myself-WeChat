#ifndef FINDRESULTDIALOG_H
#define FINDRESULTDIALOG_H

#include <QDialog>
#include "Global.h"
#include "UserData.h"
#include <QMouseEvent>

namespace Ui {
class FindResultDialog;
}

class FindResultDialog : public QDialog
{
    Q_OBJECT

public:
    explicit FindResultDialog(QWidget *parent = nullptr);
    ~FindResultDialog();
    void setSearchInfo(std::shared_ptr<SearchInfo> si);
    void setAddBtnText();
protected:
    virtual void mousePressEvent(QMouseEvent* e) override;
    virtual void mouseMoveEvent(QMouseEvent* e) override;

private slots:
    void on_add_user_btn_clicked();

private:
    void handleMoveDialog(QPoint& pos, QMouseEvent* e);

    Ui::FindResultDialog *ui;
    std::shared_ptr<SearchInfo> _si;
    QPoint offset;
};

#endif // FINDRESULTDIALOG_H
