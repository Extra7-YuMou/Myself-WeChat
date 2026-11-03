#ifndef BUBBLEFRAME_H
#define BUBBLEFRAME_H

#include <QFrame>
#include "global.h"
#include <QHBoxLayout>

class BubbleFrame : public QFrame
{
    Q_OBJECT
public:
    BubbleFrame(ChatRole role, QWidget* parent = nullptr);
    void setWidget(QWidget* w);
    void setMargin(int margin);
    ChatRole getChatRole(){return m_role;}
protected:
    void paintEvent(QPaintEvent* e);
private:
    const int WIDTH_SANJIAO = 8;
    ChatRole m_role;
    int m_margin;
    QHBoxLayout* m_pHLayout;
};

#endif // BUBBLEFRAME_H
