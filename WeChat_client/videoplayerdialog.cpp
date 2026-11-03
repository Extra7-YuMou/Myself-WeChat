#include "videoplayerdialog.h"
#include "ui_videoplayerdialog.h"
#include <QPushButton>
#include <QSlider>
#include <QVBoxLayout>

VideoPlayerDialog::VideoPlayerDialog(const QString& path, QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::VideoPlayerDialog)
{
    setWindowTitle("视频播放");
    resize(800, 450);
    setAttribute(Qt::WA_DeleteOnClose);
    _widget = new QVideoWidget(this);
    _player = new QMediaPlayer(this);
    _output = new QAudioOutput(this);
    _player->setAudioOutput(_output);
    _player->setVideoOutput(_widget);
    _output->setVolume(0.8);
    auto* playBtn = new QPushButton("播放/暂停", this);
    auto* slider = new QSlider(Qt::Horizontal, this);
    auto* lay = new QVBoxLayout(this);
    lay->addWidget(_widget, 1);
    lay->addWidget(slider);
    lay->addWidget(playBtn);
    connect(playBtn, &QPushButton::clicked, this, [this]() {
        if (_player->playbackState() == QMediaPlayer::PlayingState)
            _player->pause();
        else
            _player->play();
    });
    connect(_player, &QMediaPlayer::durationChanged, slider, &QSlider::setMaximum);
    connect(_player, &QMediaPlayer::positionChanged, slider, &QSlider::setValue);
    connect(slider, &QSlider::sliderMoved, _player, &QMediaPlayer::setPosition);
    _player->setSource(QUrl::fromLocalFile(path));
    _player->play();
}

VideoPlayerDialog::~VideoPlayerDialog()
{
    _player->stop();
}
