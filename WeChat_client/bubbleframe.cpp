#include "bubbleframe.h"
#include <QPainter>
#include <QColor>
#include <QRect>
#include <QRgb>

BubbleFrame::BubbleFrame(ChatRole role, QWidget *parent): QFrame(parent),
    m_role(role), m_margin(3)
{
    m_pHLayout = new QHBoxLayout();
    if(role == ChatRole::Other){
        m_pHLayout->setContentsMargins(WIDTH_SANJIAO + m_margin, m_margin, m_margin, m_margin);
    }else{
        m_pHLayout->setContentsMargins(m_margin, m_margin, WIDTH_SANJIAO + m_margin, m_margin);
    }
    this->setLayout(m_pHLayout);
}

void BubbleFrame::setWidget(QWidget *w)
{
    if(this->layout()->count() > 0){
        return;
    }else{
        this->layout()->addWidget(w);
    }
}

void BubbleFrame::setMargin(int margin)
{
    this->m_margin = margin;
    if(m_role == ChatRole::Other){
        this->setContentsMargins(WIDTH_SANJIAO + m_margin, m_margin, m_margin, m_margin);
    }else{
        this->setContentsMargins(m_margin, m_margin, WIDTH_SANJIAO + m_margin, m_margin);
    }
}

void BubbleFrame::paintEvent(QPaintEvent *e)
{
    QPainter painter(this);
    painter.setPen(Qt::NoPen);

    if(m_role == ChatRole::Other){
        QColor color(224, 224, 224);
        painter.setBrush(color);
        QRect rect(WIDTH_SANJIAO, 0, this->width() - WIDTH_SANJIAO, this->height());
        painter.drawRoundedRect(rect, 5, 5);
        QPointF point[3] = {
            QPointF(rect.x(), 12),
            QPointF(rect.x(), 12 + WIDTH_SANJIAO + 2),
            QPointF(rect.x() - WIDTH_SANJIAO, 12 + WIDTH_SANJIAO - WIDTH_SANJIAO / 2)
        };
        painter.drawPolygon(point, 3);
    }else{
        QColor color(158,234,106);
        painter.setBrush(color);
        QRect rect(0, 0, this->width() - WIDTH_SANJIAO, this->height());
        painter.drawRoundedRect(rect, 5, 5);
        QPointF point[3] = {
            QPointF(rect.x() + rect.width(), 12),
            QPointF(rect.x() + rect.width(), 12 + WIDTH_SANJIAO + 2),
            QPointF(rect.x() + rect.width() + WIDTH_SANJIAO, 10 + WIDTH_SANJIAO - WIDTH_SANJIAO / 2)
        };
        painter.drawPolygon(point, 3);
    }

    return QFrame::paintEvent(e);
}
