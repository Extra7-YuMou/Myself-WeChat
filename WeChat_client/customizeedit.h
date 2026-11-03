#ifndef CUSTOMIZEEDIT_H
#define CUSTOMIZEEDIT_H

#include <QLineEdit>

class CustomizeEdit : public QLineEdit{
    Q_OBJECT
public:
    CustomizeEdit(QWidget* parent = nullptr);
    void setLimitLength(int max_len);
protected:
    void focusOutEvent(QFocusEvent *) override;

private:
    void limitTextLength();
    int _max_len;

signals:
    void sig_focus_out();
};

#endif // CUSTOMIZEEDIT_H
