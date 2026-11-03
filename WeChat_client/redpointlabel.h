#ifndef REDPOINTLABEL_H
#define REDPOINTLABEL_H

#include "clicklabel.h"

class RedPointLabel : public ClickLabel
{
    Q_OBJECT
public:
    RedPointLabel(QWidget* parent = nullptr);
    void setUnReadCount(int unReadCount);
protected:
    virtual void paintEvent(QPaintEvent* e);

private:
    QSize calculateBadgeSize() const;

    int m_UnReadCount;
};

#endif // REDPOINTLABEL_H
