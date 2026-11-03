#include "messagedelegate.h"
#include "UserMgr.h"
#include <QFontMetricsF>
#include <QTextDocument>
#include <QDir>
#include <QCoreApplication>
#include <QStandardPaths>
#include <QRegularExpression>
#include <QFileInfo>

namespace {
constexpr int kHeadSize = 42;
constexpr int kMargin = 8;
constexpr int kNickHeight = 20;
constexpr int kBubbleMaxWidth = 250;
constexpr int kSanjiaoWidth = 8;
constexpr int kContentPad = 5;
constexpr int kTopMargin = kMargin * 2;
}

MessageDelegate::MessageDelegate(QObject *parent)
    : QStyledItemDelegate{parent}
{}

// bubbleRect 含三角占位；内容只能画在圆角矩形区域内
static QRect contentRectOfBubble(const QRect& bubbleRect, bool isSelf)
{
    if (isSelf) {
        // 右侧留给三角
        return bubbleRect.adjusted(kContentPad, kContentPad,
                                   -(kSanjiaoWidth + kContentPad), -kContentPad);
    }
    // 左侧留给三角
    return bubbleRect.adjusted(kSanjiaoWidth + kContentPad, kContentPad,
                               -kContentPad, -kContentPad);
}

void MessageDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    painter->save();
    painter->fillRect(option.rect, option.widget->palette().window());

    auto msg_id = index.data(MessageModel::RoleMessageId).toLongLong();
    auto content = index.data(MessageModel::RoleContent).toString();
    auto type = index.data(MessageModel::RoleMsgType).toInt();
    auto send_uid = index.data(MessageModel::RoleSendUid).toInt();

    auto self_uid = UserMgr::GetInstance()->getUid();
    bool isSelf = (self_uid == send_uid);

    QFont font;
    font.setPointSize(11);
    QFontMetricsF fm(font);

    QRect rect = option.rect;
    QRect headRect;
    QRect nickRect;
    QRect bubbleRect;
    QRect statusRect;
    QSize imgSize;
    QString total_path;

    if (isSelf) {
        headRect = QRect(rect.right() - kHeadSize - kMargin + 1,
                         rect.top() + kTopMargin,
                         kHeadSize, kHeadSize);
        nickRect = QRect(headRect.left() - kMargin - 150,
                         rect.top() + kTopMargin,
                         150, kNickHeight);
    } else {
        headRect = QRect(kMargin, rect.top() + kTopMargin, kHeadSize, kHeadSize);
        nickRect = QRect(headRect.right() + kMargin,
                         rect.top() + kTopMargin,
                         150, kNickHeight);
    }

    if (type == ChatMsgType::Text) {
        // 气泡总宽 = 正文可用宽 + 左右内边距 + 三角占位
        const int textInnerWidth = qMin(kBubbleMaxWidth - kSanjiaoWidth - 2 * kContentPad,
                                        int(fm.horizontalAdvance(content)) + 1);
        const int bubbleWidth = textInnerWidth + 2 * kContentPad + kSanjiaoWidth;

        if (isSelf) {
            bubbleRect = QRect(headRect.left() - kMargin - bubbleWidth,
                               nickRect.bottom() + kMargin,
                               bubbleWidth, 0);
        } else {
            bubbleRect = QRect(headRect.right() + kMargin,
                               nickRect.bottom() + kMargin,
                               bubbleWidth, 0);
        }
    } else if (type == ChatMsgType::Img) {
        total_path = index.data(MessageModel::RoleImgTotalPath).toString();
        imgSize = index.data(MessageModel::RolePixmapSize).toSize();
        const int bubbleWidth = imgSize.width() + 2 * kContentPad + kSanjiaoWidth;
        const int bubbleHeight = imgSize.height() + 2 * kContentPad;

        if (isSelf) {
            bubbleRect = QRect(headRect.left() - kMargin - bubbleWidth,
                               nickRect.bottom() + kMargin,
                               bubbleWidth, bubbleHeight);
            statusRect = QRect(bubbleRect.left() - 30,
                               bubbleRect.center().y() - 12,
                               25, 25);
            _status_rects[msg_id] = statusRect;
        } else {
            bubbleRect = QRect(headRect.right() + kMargin,
                               nickRect.bottom() + kMargin,
                               bubbleWidth, bubbleHeight);
        }
    }else if(type == ChatMsgType::Video){
        QString coverPath = index.data(MessageModel::RoleVideoCoverPath).toString();
        imgSize = index.data(MessageModel::RolePixmapSize).toSize();

        const int bubbleWidth = imgSize.width() + 2 * kContentPad + kSanjiaoWidth;
        const int bubbleHeight = imgSize.height() + 2 * kContentPad;

        if (isSelf) {
            bubbleRect = QRect(headRect.left() - kMargin - bubbleWidth,
                               nickRect.bottom() + kMargin,
                               bubbleWidth, bubbleHeight);
            statusRect = QRect(bubbleRect.left() - 30,
                               bubbleRect.center().y() - 12,
                               25, 25);
            _status_rects[msg_id] = statusRect;
        } else {
            bubbleRect = QRect(headRect.right() + kMargin,
                               nickRect.bottom() + kMargin,
                               bubbleWidth, bubbleHeight);
        }
    }

    QString iconPath;
    QString nick;
    if (isSelf) {
        iconPath = UserMgr::GetInstance()->getIcon();
        nick = UserMgr::GetInstance()->getNick();
    } else {
        auto info = UserMgr::GetInstance()->GetFriendList().value(send_uid);
        if (info) {
            iconPath = info->_icon;
            nick = info->_nick;
        }
    }

    drawHeadIcon(painter, headRect, iconPath);
    drawNick(painter, nickRect, nick, isSelf);

    if (type == ChatMsgType::Text) {
        QRect contentRect = contentRectOfBubble(QRect(bubbleRect.topLeft(),
                                                      QSize(bubbleRect.width(), 1000)),
                                                isSelf);

        QTextDocument doc;
        doc.setDefaultFont(font);
        doc.setPlainText(content);
        doc.setTextWidth(contentRect.width());
        const int bubbleHeight = int(doc.size().height()) + 2 * kContentPad;
        bubbleRect.setHeight(bubbleHeight);
        contentRect = contentRectOfBubble(bubbleRect, isSelf);

        drawBubble(painter, bubbleRect, isSelf);

        painter->save();
        painter->translate(contentRect.topLeft());
        doc.drawContents(painter, QRectF(QPointF(0, 0), contentRect.size()));
        painter->restore();
    } else if (type == ChatMsgType::Img) {
        drawBubble(painter, bubbleRect, isSelf);
        QRect contentRect = contentRectOfBubble(bubbleRect, isSelf);
        QPixmap pix(total_path);
        if (!pix.isNull()) {
            pix = pix.scaled(imgSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);
            drawImage(painter, contentRect, pix);
        }
        if (isSelf) {
            drawUnReciveStatusBubble(painter, _status_rects[msg_id]);
        }
    }else if (type == ChatMsgType::Video) {
        drawBubble(painter, bubbleRect, isSelf);
        QRect contentRect = contentRectOfBubble(bubbleRect, isSelf);
        // 1) 封面
        QPixmap cover(index.data(MessageModel::RoleVideoCoverPath).toString());
        if (cover.isNull())
            cover = QPixmap(":/images/video_chat_normal.png");
        cover = cover.scaled(contentRect.size(), Qt::KeepAspectRatioByExpanding,
                             Qt::SmoothTransformation);
        drawImage(painter, contentRect, cover);
        // 2) 半透明圆 + 三角 ▶
        const int r = 22;
        QPoint c = contentRect.center();
        painter->setBrush(QColor(0,0,0,120));
        painter->setPen(Qt::NoPen);
        painter->drawEllipse(c, r, r);
        painter->setBrush(Qt::white);
        QPolygon play;
        play << QPoint(c.x()-6, c.y()-10)
             << QPoint(c.x()-6, c.y()+10)
             << QPoint(c.x()+10, c.y());
        painter->drawPolygon(play);
        // 3) 时长（用你们已有的 formatDuration）
        int sec = index.data(MessageModel::RoleVideoDuration).toInt();
        QString dur = formatDuration(sec);  // "00:02"
        painter->setPen(Qt::white);
        painter->drawText(contentRect.adjusted(0,0,-6,-4),
                          Qt::AlignRight | Qt::AlignBottom, dur);
    }

    painter->restore();
}

QSize MessageDelegate::sizeHint(const QStyleOptionViewItem &option,
                                const QModelIndex &index) const
{
    int type = index.data(MessageModel::RoleMsgType).toInt();
    int contentHeight = 0;

    if (type == ChatMsgType::Text) {
        QString text = index.data(MessageModel::RoleContent).toString();
        QFont font;
        font.setPointSize(11);
        QFontMetricsF fm(font);
        const int textInnerWidth = qMin(kBubbleMaxWidth - kSanjiaoWidth - 2 * kContentPad,
                                        int(fm.horizontalAdvance(text)) + 1);
        QTextDocument doc;
        doc.setDefaultFont(font);
        doc.setPlainText(text);
        doc.setTextWidth(textInnerWidth);
        contentHeight = int(doc.size().height()) + 2 * kContentPad;
    } else if (type == ChatMsgType::Img) {
        QSize imgSize = index.data(MessageModel::RolePixmapSize).toSize();
        contentHeight = imgSize.height() + 2 * kContentPad;
    }else if (type == ChatMsgType::Video) {
        QSize s = index.data(MessageModel::RolePixmapSize).toSize();
        contentHeight = s.height() + 2 * kContentPad;
    }

    int totalHeight = kTopMargin + kNickHeight + kMargin + contentHeight + kMargin;
    totalHeight = qMax(totalHeight, kHeadSize + kTopMargin + kMargin);
    return QSize(option.rect.width(), totalHeight);
}

void MessageDelegate::drawBubble(QPainter *painter, const QRect &rect, bool isSelf) const
{
    painter->setPen(Qt::NoPen);
    painter->setRenderHint(QPainter::Antialiasing);

    if (isSelf) {
        painter->setBrush(QColor(158, 234, 106));
        // 圆角主体在左侧，右侧留给三角
        QRect body(rect.left(), rect.top(),
                   rect.width() - kSanjiaoWidth, rect.height());
        painter->drawRoundedRect(body, 5, 5);
        QPolygon triangle;
        triangle << QPoint(body.right(), rect.top() + 12)
                 << QPoint(body.right() + kSanjiaoWidth, rect.top() + 12 + kSanjiaoWidth / 2)
                 << QPoint(body.right(), rect.top() + 12 + kSanjiaoWidth + 2);
        painter->drawPolygon(triangle);
    } else {
        painter->setBrush(QColor(224, 224, 224));
        // 圆角主体在右侧，左侧留给三角；宽度必须扣掉三角
        QRect body(rect.left() + kSanjiaoWidth, rect.top(),
                   rect.width() - kSanjiaoWidth, rect.height());
        painter->drawRoundedRect(body, 5, 5);
        QPolygon triangle;
        triangle << QPoint(body.left(), rect.top() + 12)
                 << QPoint(body.left() - kSanjiaoWidth, rect.top() + 12 + kSanjiaoWidth / 2)
                 << QPoint(body.left(), rect.top() + 12 + kSanjiaoWidth + 2);
        painter->drawPolygon(triangle);
    }
}

void MessageDelegate::drawUnReciveStatusBubble(QPainter *painter, const QRect &rect) const
{
    QPixmap pix(":/images/un_read.png");
    pix = pix.scaled(QSize(25, 25), Qt::KeepAspectRatio, Qt::SmoothTransformation);

    painter->save();
    painter->setClipRect(rect);
    painter->drawPixmap(rect, pix);
    painter->restore();
}

void MessageDelegate::drawHeadIcon(QPainter *painter, const QRect &rect, const QString &icon) const
{
    QRegularExpression regexp("^head_(\\d+)\\.jpg$");
    QRegularExpressionMatch match = regexp.match(icon);
    if (match.hasMatch()) {
        QString appPath = QCoreApplication::applicationDirPath();
        QString iconPath = QDir::toNativeSeparators(appPath + QDir::separator() +
                                                    "static" + QDir::separator() + icon);
        QPixmap pix(iconPath);
        painter->save();
        painter->setClipRect(rect);
        painter->drawPixmap(rect, pix.scaled(rect.size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
        painter->restore();
    } else {
        QString standardDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
        QDir avatarsDir = standardDir + QDir::separator() + "avatars";

        if (avatarsDir.exists()) {
            QString iconPath = avatarsDir.filePath(QFileInfo(icon).fileName());
            QPixmap pix(iconPath);
            if (pix.isNull()) {
                QString appPath = QCoreApplication::applicationDirPath();
                QString defaultIcon = QDir::toNativeSeparators(appPath + QDir::separator() +
                                                               "static" + QDir::separator() + "WeChat.jpg");
                QPixmap tmp_pix(defaultIcon);
                painter->save();
                painter->setClipRect(rect);
                painter->drawPixmap(rect, tmp_pix.scaled(rect.size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
                painter->restore();
                return;
            }
            painter->save();
            painter->setClipRect(rect);
            painter->drawPixmap(rect, pix.scaled(rect.size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
            painter->restore();
        } else {
            QString appPath = QCoreApplication::applicationDirPath();
            QString iconPath = QDir::toNativeSeparators(appPath + QDir::separator() +
                                                        "static" + QDir::separator() + "Wechat.jpg");
            QPixmap pix(iconPath);
            painter->save();
            painter->setClipRect(rect);
            painter->drawPixmap(rect, pix.scaled(rect.size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
            painter->restore();
        }
    }
}

void MessageDelegate::drawNick(QPainter *painter, const QRect &rect, const QString &nick, bool isSelf) const
{
    if (isSelf) {
        painter->drawText(rect, Qt::AlignRight | Qt::AlignVCenter, nick);
    } else {
        painter->drawText(rect, Qt::AlignLeft | Qt::AlignVCenter, nick);
    }
}

void MessageDelegate::drawImage(QPainter *painter, const QRect &rect, const QPixmap& pix) const
{
    painter->save();
    painter->setClipRect(rect);
    painter->drawPixmap(rect, pix);
    painter->restore();
}
