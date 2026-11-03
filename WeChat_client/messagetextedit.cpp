#include "messagetextedit.h"
#include <QCryptographicHash>
#include <QFileInfo>
#include <QFile>
#include <QDir>
#include <QRegularExpression>
#include <QDebug>

MessageTextEdit::MessageTextEdit(QWidget *parent): QTextEdit(parent){}

QVector<MsgInfo> MessageTextEdit::getMsgList()
{
    mGetMsgList.clear();
    QString doc = this->document()->toPlainText();
    QString text = "";
    int indexUrl = 0;
    int count = mMsgList.size();

    for(int index = 0; index < doc.size(); index++){
        if(doc[index] == QChar::ObjectReplacementCharacter){
            if(!text.isEmpty()){
                QPixmap pix;
                insertList(mGetMsgList, ChatMsgType::Text, text, pix);
                text.clear();
            }
            while(indexUrl < count){
                MsgInfo info = mMsgList.at(indexUrl);
                if(this->document()->toHtml().contains(info._content, Qt::CaseInsensitive)){
                    indexUrl++;
                    mGetMsgList.append(info);
                    break;
                }
                indexUrl++;
            }
        }else{
            text.append(doc[index]);
        }
    }
    if(!text.isEmpty()){
        insertList(mGetMsgList, ChatMsgType::Text, text, QPixmap());
    }

    mMsgList.clear();
    this->clear();
    return mGetMsgList;
}

void MessageTextEdit::insertFromUrl(const QString &url)
{
    if(isImage(url)){
        insertImage(url);
    }else if(isVideo(url)){
        insertVideo(url);
    }else{
        insertTextFile(url);
    }
}

void MessageTextEdit::dragEnterEvent(QDragEnterEvent *e)
{
    if(e->source() == this){
        e->ignore();
    }else{
        e->accept();
    }
}

void MessageTextEdit::dropEvent(QDropEvent *e)
{
    insertFromMimeData(e->mimeData());
    e->accept();
}

void MessageTextEdit::keyPressEvent(QKeyEvent *e)
{
    // 检查按下的键是否为Enter 或者 return
    if(e->key() == Qt::Key_Return || e->key() == Qt::Key_Enter){
        // 如果没有按下Shift修饰键，则视为发送消息
        if(!(e->modifiers() & Qt::ShiftModifier)){
            emit send();
            e->accept();
            return;
        }
    }
    QTextEdit::keyPressEvent(e);
}

bool MessageTextEdit::isImage(const QString &url)
{
    QString imageFormat = "bmp,jpg,png,tif,gif,pcx,tga,exif,fpx,svg,psd,cdr,pcd,dxf,ufo,eps,ai,raw,wmf,webp";

    QFileInfo info(url);
    QString suffix = info.suffix();
    if(imageFormat.contains(suffix)){
        return true;
    }
    return false;
}

bool MessageTextEdit::isVideo(const QString &url)
{
    QString videoFormat = "mp4";
    QFileInfo info(url);
    QString suffix = info.suffix();
    if(videoFormat.contains(suffix)){
        return true;
    }
    return false;
}

void MessageTextEdit::insertImage(const QString &url)
{
    QImage image(url);

    if(image.width() > 120 || image.height() > 80){
        if(image.width() > image.height()){
            image = image.scaledToWidth(120, Qt::SmoothTransformation);
        }else{
            image = image.scaledToHeight(80, Qt::SmoothTransformation);
        }
    }

    QTextCursor cursor = this->textCursor();
    cursor.insertImage(image, url);
    insertList(mMsgList, ChatMsgType::Img, url, QPixmap::fromImage(image));
}

void MessageTextEdit::insertTextFile(const QString &url)
{
    QFileInfo info(url);
    if(info.isDir()){
        QMessageBox::information(nullptr, "提示", "只允许拖拽单个文件");
        return;
    }

    if(info.size() > 100 * 1024 * 1024){
        QMessageBox::information(nullptr, "提示", "单个文件大小不得超过100MB");
        return;
    }

    QTextCursor cursor = this->textCursor();
    QPixmap pix(":/images/file.png");
    pix = pix.scaled(QSize(120, 80), Qt::KeepAspectRatio, Qt::SmoothTransformation);
    cursor.insertImage(pix.toImage(), url);
    insertList(mMsgList, ChatMsgType::File, url, pix);
}

void MessageTextEdit::insertVideo(const QString &url)
{
    // 只在这里生成一次 UUID，并抽一次封面；发送时复用 MsgInfo 中的字段
    const QString uniqueFileName = generateUniqueFileName(QFileInfo(url).fileName());
    QString coverName = uniqueFileName;
    coverName.replace(QRegularExpression("\\.[^.]+$"), "_cover.jpg");

    const QString coverDir = QDir(GetDownLoadFilePrefix()).filePath(QStringLiteral("video_covers"));
    QDir().mkpath(coverDir);
    const QString coverPath = QDir::toNativeSeparators(QDir(coverDir).filePath(coverName));

    const QString savedCover = makeCoverForVideo(url, coverPath);
    if (savedCover.isEmpty()) {
        qWarning() << "insertVideo: make cover failed" << url;
        QMessageBox::warning(nullptr, "提示", "无法生成视频封面，请检查 ffmpeg 或视频文件");
        return;
    }

    QImage image(savedCover);
    if (image.isNull()) {
        qWarning() << "insertVideo: cover image load failed" << savedCover;
        return;
    }
    if (image.width() > 120 || image.height() > 80) {
        if (image.width() > image.height()) {
            image = image.scaledToWidth(120, Qt::SmoothTransformation);
        } else {
            image = image.scaledToHeight(80, Qt::SmoothTransformation);
        }
    }

    QTextCursor cursor = this->textCursor();
    cursor.insertImage(image, url);

    MsgInfo info;
    info._msgFlag = ChatMsgType::Video;
    info._content = url;                 // 供 getMsgList 用 HTML 匹配
    info._total_path = url;              // 本地原视频路径
    info._name = uniqueFileName;         // 发给服务器的视频名
    info._cover_name = coverName;        // 发给服务器的封面名
    info._cover_path = savedCover;       // 本地封面绝对路径
    info._pixmap = QPixmap::fromImage(image);
    mMsgList.append(info);
}

void MessageTextEdit::insertList(QVector<MsgInfo> &list, const ChatMsgType &flag, const QString &content, QPixmap pix)
{
    MsgInfo info;
    // info._msgFlag = flag;
    // info._content = content;
    // info._pixmap = pix;
    // list.append(info);

    info._msgFlag = flag;
    if(flag == ChatMsgType::Text){
        info._content = content;
    }else if(flag == ChatMsgType::Img){
        QFileInfo file_info(content);
        QString file_name = file_info.fileName();
        qDebug() << "file name: " << file_name;
        info._name = file_name;
        info._pixmap = pix;
        info._total_path = content;
        info._content = content;
    }
    // Video 在 insertVideo 中直接构造 MsgInfo，不再走这里

    list.append(info);
}

QStringList MessageTextEdit::getUrl(const QString &text)
{
    QStringList urls;
    if(text.isEmpty())return urls;

    QStringList urlList = text.split("\n");
    for(auto& url : urlList){
        QStringList temp = url.split("///");
        if(temp.size() > 1){
            urls.append(temp.at(1));
        }
    }
    return urls;
}

void MessageTextEdit::insertFromMimeData(const QMimeData *mimeData)
{
    QStringList urls = getUrl(mimeData->text());
    if(urls.isEmpty()) return;

    for(auto& url : urls){
        if(isImage(url)){
            insertImage(url);
        }else if(isVideo(url)){
            insertVideo(url);
        }else{
            insertTextFile(url);
        }
    }
}
