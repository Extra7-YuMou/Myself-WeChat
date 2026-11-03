#ifndef PICTUREBUBBLE_H
#define PICTUREBUBBLE_H

#include "bubbleframe.h"
#include <QLabel>

#define MAX_PIC_WIDTH 200
#define MAX_PIC_HEIGHT 120

class PictureBubble : public BubbleFrame
{
public:
    PictureBubble(ChatRole role, const QPixmap& picture, QWidget* parent = nullptr);
};

#endif // PICTUREBUBBLE_H
