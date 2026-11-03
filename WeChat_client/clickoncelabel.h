#ifndef CLICKONCELABEL_H
#define CLICKONCELABEL_H

#include <QLabel>
#include <QMouseEvent>

class ClickOnceLabel : public QLabel
{
    Q_OBJECT
public:
    ClickOnceLabel(QWidget* parent = nullptr);

protected:
    void mousePressEvent(QMouseEvent* e);
signals:
    void clicked(QString);
};

#endif // CLICKONCELABEL_H
