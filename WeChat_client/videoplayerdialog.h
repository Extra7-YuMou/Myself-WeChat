#ifndef VIDEOPLAYERDIALOG_H
#define VIDEOPLAYERDIALOG_H

#include <QDialog>
#include <QMediaPlayer>
#include <QAudioOutput>
#include <QUrl>
#include <QVideoWidget>

namespace Ui {
class VideoPlayerDialog;
}

class VideoPlayerDialog : public QDialog
{
    Q_OBJECT

public:
    explicit VideoPlayerDialog(const QString& path, QWidget *parent = nullptr);
    ~VideoPlayerDialog();

private:
    Ui::VideoPlayerDialog *ui;

    QMediaPlayer* _player = nullptr;
    QAudioOutput* _output = nullptr;
    QVideoWidget* _widget = nullptr;
};

#endif // VIDEOPLAYERDIALOG_H
