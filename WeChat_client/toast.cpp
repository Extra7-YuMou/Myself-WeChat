#include "toast.h"
// toast.cpp
Toast::Toast(QWidget *parent) : QWidget(parent)
{
    setWindowFlags(Qt::Tool | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
    setAttribute(Qt::WA_TranslucentBackground);
    setStyleSheet("background: rgba(0,0,0,180); color: white; border-radius:8px;");

    m_label = new QLabel(this);
    m_label->setAlignment(Qt::AlignCenter);
    QHBoxLayout *layout = new QHBoxLayout(this);
    layout->addWidget(m_label);

    m_animation = new QPropertyAnimation(this, "pos");
    m_animation->setDuration(300);
    m_animation->setEasingCurve(QEasingCurve::OutQuad);
}

void Toast::showMessage(const QString &msg)
{
    if (msg.isEmpty()) return;

    // 确保有父窗口，否则基于屏幕居中
    QWidget *parent = parentWidget();
    if (!parent) {
        // 回退到屏幕中央
        QRect screen = QGuiApplication::primaryScreen()->geometry();
        int tx = (screen.width() - width()) / 2;
        int ty = (screen.height() - height()) / 2;
        move(tx, ty);
        show();
        return;
    }

    QSize size = calculateSize(msg);
    m_label->setFixedSize(size);
    m_label->setText(msg);
    adjustSize();

    QPoint parentPos = parent->mapToGlobal(QPoint(0, 0));
    // 水平居中
    int tx = parentPos.x() + (parent->width())/2;
    int ty = parentPos.y() + this->height() + 20;

    // 动画：从上方滑入
    QPoint startPos(tx, ty - height());
    QPoint targetPos(tx, ty);

    m_animation->setStartValue(startPos);
    m_animation->setEndValue(targetPos);
    move(startPos);
    show();
    m_animation->start();
}

void Toast::hide()
{
    this->close();
}

QSize Toast::calculateSize(const QString &text)
{
    QFont font = m_label->font();
    QFontMetricsF fm(font);
    int width = fm.horizontalAdvance(text);
    int height = fm.height();
    return QSize(width + 20, height + 10);
}