#ifndef GROUPTIPITEM_H
#define GROUPTIPITEM_H

#include <QWidget>

namespace Ui {
class GroupTipItem;
}

class GroupTipItem : public QWidget
{
    Q_OBJECT

public:
    explicit GroupTipItem(QWidget *parent = nullptr);
    ~GroupTipItem();
    QSize sizeHint() const override{
        return QSize(250, 25);
    }
    void setTip(const QString& tip);
private:
    Ui::GroupTipItem *ui;
};

#endif // GROUPTIPITEM_H
