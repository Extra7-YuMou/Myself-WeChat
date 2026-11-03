#include "picturebubble.h"

PictureBubble::PictureBubble(ChatRole role, const QPixmap& picture, QWidget *parent) : BubbleFrame(role, parent)
{
    QLabel* pic = new QLabel;
    pic->setScaledContents(true);
    QPixmap pix = picture.scaled(QSize(MAX_PIC_WIDTH, MAX_PIC_HEIGHT), Qt::KeepAspectRatio);
    pic->setPixmap(pix);
    this->setWidget(pic);

    int margin_top = this->layout()->contentsMargins().top();
    int margin_left = this->layout()->contentsMargins().left();
    int margin_right = this->layout()->contentsMargins().right();
    setFixedSize(MAX_PIC_WIDTH + margin_left + margin_right, MAX_PIC_HEIGHT + margin_top * 2);
}
