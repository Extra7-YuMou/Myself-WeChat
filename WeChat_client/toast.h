#ifndef TOAST_H
#define TOAST_H

#include <QWidget>
#include <QLabel>
#include <QString>
#include <QHBoxLayout>
#include <QFontMetricsF>
#include <QTimer>
#include <QPropertyAnimation>
#include <QGuiApplication>
#include <QScreen>

class Toast : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(QPoint pos READ pos WRITE move)
public:
    explicit Toast(QWidget *parent = nullptr);
    void showMessage(const QString& msg);
    void hide();
private:
    QSize calculateSize(const QString& text);

    QTimer* _timer;
    QLabel* m_label;
    QPropertyAnimation* m_animation;
signals:
};

#endif // TOAST_H
