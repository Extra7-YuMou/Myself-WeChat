#ifndef CHATVIEW_H
#define CHATVIEW_H
#include <QWidget>
#include <QVBoxLayout>
#include <QScrollArea>

/******************************************************************************
 *
 * @file            chatview.h
 * @brief           聊天布局
 * @description
 *
 * @author          YuHaoNan
 * @date            2026/04/20
 * @history
 *****************************************************************************/
class ChatView : public QWidget
{
    Q_OBJECT
public:
    explicit ChatView(QWidget *parent = nullptr);
    ~ChatView();
    // 尾插
    void appendChatItem(QWidget* item);
    // 头插
    void prependChatItem(QWidget* item);
    // 中间插
    void insertChatItem(QWidget* before, QWidget* item);

protected:
    bool eventFilter(QObject* watched, QEvent* e);
    void paintEvent(QPaintEvent* e);
private:
    // void initStyle();

    QVBoxLayout* m_pV1;
    QScrollArea* m_pScrollArea;
    bool isAppend;
signals:

private slots:
    void onVScrollBarMoved(int min, int max);
};

#endif // CHATVIEW_H
