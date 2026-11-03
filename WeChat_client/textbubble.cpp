#include "textbubble.h"
#include <QScrollBar>
#include <QTextBlock>
#include <QFontMetricsF>

TextBubble::TextBubble(ChatRole role, const QString &text, QWidget *parent)
    : BubbleFrame(role, parent)
{
    m_textEdit = new QTextEdit;
    m_textEdit->setReadOnly(true);
    m_textEdit->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_textEdit->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    QFont font("Microsoft YaHei");
    font.setPointSize(12);
    m_textEdit->setFont(font);
    m_textEdit->installEventFilter(this);
    setPlainText(text);
    this->setWidget(m_textEdit);
    initStyle();
}

bool TextBubble::eventFilter(QObject *watched, QEvent *e)
{
    if(watched == m_textEdit && e->type() == QEvent::Paint){
        adjustTextHeight();
    }
    return BubbleFrame::eventFilter(watched, e);
}

void TextBubble::initStyle()
{
    this->m_textEdit->setStyleSheet("QTextEdit{border:none; background-color:transparent;}");
}

void TextBubble::setPlainText(const QString &text)
{
    m_textEdit->setPlainText(text);

    qreal doc_margin = m_textEdit->document()->documentMargin();
    int margin_left = this->layout()->contentsMargins().left();
    int margin_right = this->layout()->contentsMargins().right();
    QFontMetricsF fm(m_textEdit->font());
    QTextDocument* doc = m_textEdit->document();
    int maxW = 0;

    QString plainText;
    for(QTextBlock it = doc->begin(); it != doc->end(); it = it.next()){
        if(it.text().length() == 0){
            continue;
        }
        plainText += it.text() + (it.next() == doc->end() ? "" : "\n");

        int textW = int(fm.horizontalAdvance(it.text() + " "));
        maxW = maxW > textW ? maxW : textW;
    }

    m_textEdit->setPlainText(plainText);
    setMaximumWidth(maxW + doc_margin * 2 + margin_left + margin_right);
}

void TextBubble::adjustTextHeight()
{
    qreal doc_margin = m_textEdit->document()->documentMargin();
    int margin_top = this->layout()->contentsMargins().top();
    QTextDocument* doc = m_textEdit->document();
    int textH = 0;
    for(QTextBlock it = doc->begin(); it != doc->end(); it = it.next()){
        QTextLayout* layout = it.layout();
        QRectF rectF = layout->boundingRect();
        textH += rectF.height();
    }
    setMaximumHeight(textH + doc_margin * 2 + margin_top * 2);
}