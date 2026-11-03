#include "global.h"
#include <QFileInfo>
#include <QFile>
#include <QDir>
#include <QCoreApplication>
#include <QProcess>
#include <QPixmap>
#include <QDebug>

QString gate_url_prefix = "";
const QString get_verify_success = "验证码已发送至邮箱，请注意查收!";

std::function<void(QWidget*)> repolish = [](QWidget* w){
    w->style()->unpolish(w);
    w->style()->polish(w);
};

std::function<QString(QString)> xorString = [](QString input){
    QString result = input;
    int length = input.length();
    length %= 255;
    for(int i = 0; i < length; i++){
        result[i] = QChar(static_cast<ushort>(input[i].unicode() ^ static_cast<ushort>(length)));
    }
    return result;
};

QString generateUniqueFileName(const QString &originalName){
    QString uuid = QUuid::createUuid().toString(QUuid::WithoutBraces);
    // 例如 "image.jpg" -> "image_550e8400-e29b-41d4-a716-446655440000.jpg"
    QFileInfo info(originalName);
    return QString("%1_%2.%3")
        .arg(info.baseName())
        .arg(uuid)
        .arg(info.suffix());
}

QString GetDownLoadFilePrefix()
{
    return QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
}

QString GetLocalFilePrefix(qint64 thread_id)
{
    return QDir::toNativeSeparators(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation)
                                    + QDir::separator() + "thread_id_" + QString::number(thread_id)
                                    + QDir::separator());
}

QString ffmpegPath()
{
    // 优先使用随包拷贝到输出目录的 ffmpeg
    const QString bundled = QDir::toNativeSeparators(
        QCoreApplication::applicationDirPath() + "/static/ffmpeg/ffmpeg.exe");
    if (QFile::exists(bundled)) {
        return bundled;
    }

    // 其次尝试工程相对路径（开发时直接跑也可能用到）
    const QString local = QDir::toNativeSeparators(
        QCoreApplication::applicationDirPath() + "/ffmpeg.exe");
    if (QFile::exists(local)) {
        return local;
    }

    // 最后走 PATH 里的 ffmpeg
    return QStringLiteral("ffmpeg");
}

static bool runFfmpegExtractCover(const QString& ffmpeg, const QString& videoPath,
                                  const QString& coverPath, const QString& seekSec)
{
    QStringList args;
    args << "-hide_banner" << "-loglevel" << "error"
         << "-y"
         << "-ss" << seekSec
         << "-i" << QDir::toNativeSeparators(videoPath)
         << "-frames:v" << "1"
         << "-vf" << "scale=320:-2"
         << "-q:v" << "3"
         << QDir::toNativeSeparators(coverPath);

    QProcess p;
    p.setProcessChannelMode(QProcess::MergedChannels);
    p.start(ffmpeg, args);

    if (!p.waitForStarted(3000)) {
        qWarning() << "ffmpeg start failed:" << ffmpeg << p.errorString();
        return false;
    }

    // 大视频也通常几秒内结束；超时直接杀掉，避免卡死 UI
    if (!p.waitForFinished(15000)) {
        p.kill();
        p.waitForFinished(2000);
        qWarning() << "ffmpeg extract cover timeout:" << videoPath;
        return false;
    }

    if (p.exitStatus() != QProcess::NormalExit || p.exitCode() != 0) {
        qWarning() << "ffmpeg extract cover failed, code:" << p.exitCode()
                    << "out:" << p.readAll();
        return false;
    }

    return QFile::exists(coverPath) && QFileInfo(coverPath).size() > 0;
}

// 返回封面路径；失败返回空
QString extractVideoCover(const QString &videoPath, const QString &coverPath)
{
    if (!QFile::exists(videoPath)) {
        qWarning() << "extractVideoCover: video not exists" << videoPath;
        return {};
    }

    const QString ffmpeg = ffmpegPath();
    // PATH 模式下不预先检查文件是否存在
    if (ffmpeg.contains(QLatin1Char('/')) || ffmpeg.contains(QLatin1Char('\\'))) {
        if (!QFile::exists(ffmpeg)) {
            qWarning() << "extractVideoCover: ffmpeg not found at" << ffmpeg;
            return {};
        }
    }

    QDir().mkpath(QFileInfo(coverPath).absolutePath());

    // 先取 0.5s 处一帧；短视频失败再退回 0s
    if (runFfmpegExtractCover(ffmpeg, videoPath, coverPath, QStringLiteral("0.5"))) {
        return QDir::toNativeSeparators(coverPath);
    }
    if (runFfmpegExtractCover(ffmpeg, videoPath, coverPath, QStringLiteral("0"))) {
        return QDir::toNativeSeparators(coverPath);
    }

    return {};
}

QString makeCoverForVideo(const QString &videoPath, const QString &coverOutputPath)
{
    if (videoPath.isEmpty() || !QFile::exists(videoPath)) {
        qWarning() << "makeCoverForVideo: invalid video path" << videoPath;
        return {};
    }
    if (coverOutputPath.isEmpty()) {
        qWarning() << "makeCoverForVideo: empty cover output path";
        return {};
    }

    QDir().mkpath(QFileInfo(coverOutputPath).absolutePath());

    // 已有封面则直接复用
    if (QFile::exists(coverOutputPath) && QFileInfo(coverOutputPath).size() > 0) {
        return QDir::toNativeSeparators(coverOutputPath);
    }

    const QString extracted = extractVideoCover(videoPath, coverOutputPath);
    if (!extracted.isEmpty()) {
        qDebug() << "makeCoverForVideo ok:" << extracted;
        return extracted;
    }

    // ffmpeg 失败时写默认封面到同一路径，保证调用方仍能拿到一致文件名
    QPixmap fallback(QStringLiteral(":/images/video_chat_normal.png"));
    if (!fallback.isNull() && fallback.save(coverOutputPath, "JPG")) {
        qWarning() << "makeCoverForVideo: fallback cover used for" << videoPath;
        return QDir::toNativeSeparators(coverOutputPath);
    }

    qWarning() << "makeCoverForVideo failed:" << videoPath;
    return {};
}

QString ffprobePath()
{
    const QString bundled = QDir::toNativeSeparators(
        QCoreApplication::applicationDirPath() + "/static/ffmpeg/ffprobe.exe");
    if (QFile::exists(bundled)) {
        return bundled;
    }
    return QStringLiteral("ffprobe");
}

int extractVideoDuration(const QString& videoPath)
{
    if (!QFile::exists(videoPath) || !QFile::exists(ffprobePath()))
        return -1;
    QStringList args;
    args << "-v" << "error"
         << "-show_entries" << "format=duration"
         << "-of" << "csv=p=0"
         << QDir::toNativeSeparators(videoPath);
    QProcess p;
    p.setProcessChannelMode(QProcess::MergedChannels);
    p.start(ffprobePath(), args);
    if (!p.waitForFinished(8000)) {
        p.kill();
        p.waitForFinished(2000);
        return -1;
    }
    if (p.exitStatus() != QProcess::NormalExit || p.exitCode() != 0)
        return -1;
    bool ok = false;
    const double sec = QString::fromUtf8(p.readAllStandardOutput().trimmed()).toDouble(&ok);
    if (!ok || sec < 0)
        return -1;
    return qRound(sec);   // 83.456 → 83，给气泡显示 "01:23"
}

QString formatDuration(int sec)
{
    if (sec < 0) {
        sec = 0;
    }
    const int m = sec / 60;
    const int s = sec % 60;
    return QString("%1:%2").arg(m, 2, 10, QChar('0')).arg(s, 2, 10, QChar('0'));
}
