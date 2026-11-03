#ifndef TEXTBUBBLE_H
#define TEXTBUBBLE_H

#include "bubbleframe.h"
#include <QTextEdit>

class TextBubble : public BubbleFrame
{
    Q_OBJECT
public:
    TextBubble(ChatRole role, const QString& text, QWidget* parent = nullptr);
protected:
    bool eventFilter(QObject* watched, QEvent* e);
private:
    void initStyle();
    void setPlainText(const QString& text);
    void adjustTextHeight();
    QTextEdit* m_textEdit;
};

#endif // TEXTBUBBLE_H
