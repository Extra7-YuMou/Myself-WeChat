#ifndef CUSTOMIZEFILELABEL_H
#define CUSTOMIZEFILELABEL_H

#include "clicklabel.h"

class CustomizeFileLabel : public ClickLabel
{
    Q_OBJECT
public:
    CustomizeFileLabel(QWidget* parent = nullptr);

signals:
    void sig_appendFile(const QString& url);
protected:
    virtual void mousePressEvent(QMouseEvent* e);

private:
};

#endif // CUSTOMIZEFILELABEL_H
