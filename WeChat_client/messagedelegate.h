#ifndef MESSAGEDELEGATE_H
#define MESSAGEDELEGATE_H

#include <QObject>
#include <QStyledItemDelegate>
#include <QPainter>
#include "messagemodel.h"
#include <QMovie>

class MessageDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    explicit MessageDelegate(QObject *parent = nullptr);
    void paint(QPainter* painter, const QStyleOptionViewItem& option,
               const QModelIndex& index) const override;
    QSize sizeHint(const QStyleOptionViewItem &option,
                   const QModelIndex &index) const override;

private:
    void drawBubble(QPainter* painter, const QRect& rect, bool isSelf) const;
    void drawUnReciveStatusBubble(QPainter* painter, const QRect& rect) const;
    void drawHeadIcon(QPainter* painter, const QRect& rect, const QString& iconPath) const;
    void drawNick(QPainter* painter, const QRect& rect, const QString& nick, bool isSelf) const;
    void drawImage(QPainter* painter, const QRect& rect, const QPixmap& pix) const;
    // void drawVideo(QPainter* painter, const QRect& recv, const QPixmap& pix) const;

    QMap<qint64, QRect> _status_rects;
};

#endif // MESSAGEDELEGATE_H
