#include "chatview.h"
#include <QVBoxLayout>
#include <QScrollBar>
#include <QHBoxLayout>
#include <QEvent>
#include <QStyle>
#include <QStyleOption>
#include <QPainter>
#include <QTimer>

ChatView::ChatView(QWidget *parent)
    : QWidget{parent}, isAppend(false)
{
    QVBoxLayout* pMainLayout = new QVBoxLayout();
    this->setLayout(pMainLayout);

    m_pScrollArea = new QScrollArea();
    m_pScrollArea->setObjectName("chat_area");
    pMainLayout->addWidget(m_pScrollArea);
    m_pScrollArea->viewport()->setAutoFillBackground(false);

    QWidget* w = new QWidget(this);
    w->setObjectName("chat_bg");
    w->setAutoFillBackground(false);

    QVBoxLayout* pVLayout_1 = new QVBoxLayout;
    pVLayout_1->addWidget(new QWidget(), 100000);
    w->setLayout(pVLayout_1);
    m_pScrollArea->setWidget(w);
    m_pScrollArea->setFrameShape(QFrame::NoFrame);

    m_pScrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_pScrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    QScrollBar* pVScrollBar = m_pScrollArea->verticalScrollBar();
    connect(pVScrollBar, &QScrollBar::rangeChanged, this, &ChatView::onVScrollBarMoved);

    QHBoxLayout* pHLayout_2 = new QHBoxLayout;
    pHLayout_2->addWidget(pVScrollBar, 0, Qt::AlignRight);
    //m_pScrollArea->setLayout(pHLayout_2);
    pVScrollBar->setHidden(true);

    m_pScrollArea->setWidgetResizable(true);
    m_pScrollArea->installEventFilter(this);
    //initStyle();
}

ChatView::~ChatView()
{
    if(m_pScrollArea != nullptr){
        delete m_pScrollArea;
    }
}

void ChatView::appendChatItem(QWidget *item)
{
    QVBoxLayout* v1 = (qobject_cast<QVBoxLayout*>)(m_pScrollArea->widget()->layout());
    if(v1 == nullptr){
        return;
    }
    v1->insertWidget(v1->count() - 1, item);
    isAppend = true;
}

void ChatView::prependChatItem(QWidget *item)
{
    QVBoxLayout* v1 = qobject_cast<QVBoxLayout*>(m_pScrollArea->widget()->layout());
    v1->insertWidget(0, item);
    isAppend = true;
}

void ChatView::insertChatItem(QWidget* before, QWidget *item)
{
    QVBoxLayout* v1 = qobject_cast<QVBoxLayout*>(m_pScrollArea->widget()->layout());
    auto index = v1->indexOf(before);
    v1->insertWidget(index - 1, item);
    isAppend = true;
}

bool ChatView::eventFilter(QObject *watched, QEvent *e)
{
    if(e->type() == QEvent::Enter && watched == m_pScrollArea){
        m_pScrollArea->verticalScrollBar()->setHidden(m_pScrollArea->verticalScrollBar()->maximum() == 0);
    }else if(e->type() == QEvent::Leave){
        m_pScrollArea->verticalScrollBar()->setHidden(true);
    }

    return QWidget::eventFilter(watched, e);
}

void ChatView::paintEvent(QPaintEvent *event)
{
    QStyleOption opt;
    opt.initFrom(this);
    QPainter p(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
}

void ChatView::onVScrollBarMoved(int min, int max)
{
    if(isAppend){
        QScrollBar* scrollBar = m_pScrollArea->verticalScrollBar();
        scrollBar->setSliderPosition(scrollBar->maximum());

        QTimer::singleShot(500, [this](){
            isAppend = false;
        });
    }
}
