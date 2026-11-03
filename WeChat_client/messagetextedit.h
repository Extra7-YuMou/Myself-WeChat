#ifndef MESSAGETEXTEDIT_H
#define MESSAGETEXTEDIT_H

#include <QObject>
#include <QTextEdit>
#include <QMouseEvent>
#include <QApplication>
#include <QDrag>
#include <QMimeData>
#include <QMimeType>
#include <QFileInfo>
#include <QFileIconProvider>
#include <QPainter>
#include <QVector>
#include "global.h"
#include <QMessageBox>
#include <QTextTableCell>

class MessageTextEdit : public QTextEdit
{
    Q_OBJECT
public:
    explicit MessageTextEdit(QWidget* parent = nullptr);
    ~MessageTextEdit() = default;
    QVector<MsgInfo> getMsgList();
    void insertFromUrl(const QString& url);
protected:
    void dragEnterEvent(QDragEnterEvent* e);
    void dropEvent(QDropEvent* e);
    void keyPressEvent(QKeyEvent* e);

private:
    bool isImage(const QString& url);
    bool isVideo(const QString& url);
    void insertImage(const QString& url);
    void insertTextFile(const QString& url);
    void insertVideo(const QString& url);
    void insertList(QVector<MsgInfo>& list, const ChatMsgType& flag, const QString& content, QPixmap pix);
    QStringList getUrl(const QString& text);
    void insertFromMimeData(const QMimeData* mimeData);

    QVector<MsgInfo> mGetMsgList;
    QVector<MsgInfo> mMsgList;

signals:
    void send();
};

#endif // MESSAGETEXTEDIT_H
