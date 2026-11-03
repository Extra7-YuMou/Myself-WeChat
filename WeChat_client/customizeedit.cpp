#include "customizeedit.h"

CustomizeEdit::CustomizeEdit(QWidget* parent) : QLineEdit(parent){
    connect(this, &QLineEdit::textChanged, this, &CustomizeEdit::limitTextLength);
}

void CustomizeEdit::setLimitLength(int max_len){
    _max_len = max_len;
}

void CustomizeEdit::focusOutEvent(QFocusEvent *e){
    QLineEdit::focusOutEvent(e);
    emit sig_focus_out();
}

void CustomizeEdit::limitTextLength(){
    if(this->text().length() < _max_len){
        return;
    }

    QByteArray data = this->text().toUtf8();
    if(data.length() > _max_len){
        this->setText(data.left(_max_len));
    }
}



