#include "redpointlabel.h"
#include <QMouseEvent>
#include <QVBoxLayout>
#include <QString>
#include <QPainter>

RedPointLabel::RedPointLabel(QWidget *parent): ClickLabel(parent), m_UnReadCount(0)
{
    setAttribute(Qt::WA_OpaquePaintEvent, false);
}

void RedPointLabel::setUnReadCount(int unReadCount)
{
    m_UnReadCount = unReadCount;
    update();
}

void RedPointLabel::paintEvent(QPaintEvent *e)
{
    ClickLabel::paintEvent(e);

    if(m_UnReadCount <= 0){
        return;
    }

    QPainter painter(this);
    // 反锯齿，可以让图形更光滑
    painter.setRenderHint(QPainter::Antialiasing);

    QSize badgeSize = calculateBadgeSize();
    if(badgeSize.isNull()){
        return;
    }

    int x = this->width() - badgeSize.width();
    int y = 0;

    painter.setPen(Qt::NoPen);
    painter.setBrush(Qt::red);
    // 画椭圆，传入的宽度和高度相等画出的是圆形，否则是椭圆
    painter.drawEllipse(x, y, badgeSize.width(), badgeSize.height());

    QString text = (m_UnReadCount > 99) ? "99+" : QString::number(m_UnReadCount);
    painter.setPen(Qt::white);

    QRect rect(x, y, badgeSize.width(), badgeSize.height());
    painter.drawText(rect, Qt::AlignCenter, text);
}

QSize RedPointLabel::calculateBadgeSize() const
{
    if(m_UnReadCount <= 0){
        return QSize(0, 0);
    }

    QString text = (m_UnReadCount > 99) ? "99+" : QString::number(m_UnReadCount);
    QFontMetricsF fm(font());
    int textW = fm.horizontalAdvance(text);
    int textH = fm.height();

    int diamter = qMax(textH, textW);
    return QSize(diamter, diamter);
}
