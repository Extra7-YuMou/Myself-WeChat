#ifndef APPLYFRIENDDIALOG_H
#define APPLYFRIENDDIALOG_H

#include <QDialog>
#include "clicklabel.h"
#include "friendlabel.h"
#include "userdata.h"

namespace Ui {
class ApplyFriendDialog;
}

class ApplyFriendDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ApplyFriendDialog(QWidget *parent = nullptr);
    ~ApplyFriendDialog();
    void initTipLabels();
    void addTipLabel(ClickLabel*, QPoint, QPoint&, int, int);
    void setSearchInfo(std::shared_ptr<SearchInfo> si);

protected:
    bool eventFilter(QObject* o, QEvent* e);
    void mousePressEvent(QMouseEvent* e);
    void mouseMoveEvent(QMouseEvent* e);

private:
    void resetLabel();
    void addLabel(QString name);

    QPoint offset;
    Ui::ApplyFriendDialog *ui;

    // 已经创建好的标签
    QMap<QString, ClickLabel*> _add_labels;
    std::vector<QString> _add_label_keys;
    QPoint _label_point;

    // 自定义的标签
    QMap<QString, FriendLabel*> _friend_labels;
    std::vector<QString> _friend_label_keys;
    QPoint _tip_cur_point;
    std::vector<QString> _tip_data;
    std::shared_ptr<SearchInfo> _si;

    std::vector<QString> _selected_label;
private slots:
    //显示更多label标签
    void ShowMoreLabel();
    //输入label按下回车触发将标签加入展示栏
    void SlotLabelEnter();
    //点击关闭，移除展示栏好友便签
    void SlotRemoveFriendLabel(QString);
    //通过点击tip实现增加和减少好友便签
    void SlotChangeFriendLabelByTip(QString, ClickState);
    //输入框文本变化显示不同提示
    void SlotLabelTextChange(const QString& text);
    //输入框输入完成
    void SlotLabelEditFinished();
    //输入标签显示提示框，点击提示框内容后添加好友便签
    void SlotAddFirendLabelByClickTip(QString text);
    //处理确认回调
    void on_sure_btn_clicked();
    //处理取消回调
    void on_cancel_btn_clicked();
};

#endif // APPLYFRIENDDIALOG_H
