#ifndef FRIENDLABEL_H
#define FRIENDLABEL_H

#include <QFrame>

namespace Ui {
class FriendLabel;
}

class FriendLabel : public QFrame
{
    Q_OBJECT

public:
    explicit FriendLabel(QWidget *parent = nullptr);
    ~FriendLabel();
    void setText(const QString& text);
    QString text() const;
protected:
    bool eventFilter(QObject* o, QEvent* e);
private:
    Ui::FriendLabel *ui;
    QString _text;
    int _width;
    int _height;

signals:
    void close(QString);
};

#endif // FRIENDLABEL_H
