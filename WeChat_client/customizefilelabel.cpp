#include "customizefilelabel.h"
#include <QMouseEvent>
#include <QFileDialog>
#include <QMimeDataBase>
#include <QMimeType>

CustomizeFileLabel::CustomizeFileLabel(QWidget *parent) : ClickLabel(parent)
{

}

void CustomizeFileLabel::mousePressEvent(QMouseEvent *e)
{
    if(e->button() == Qt::LeftButton){
        QString filter = "所有文件(*.*);;"
                         "图片文件(*.bmp *.jpg *.png *.tif *.gif *.pcx *.tga *.exif *.fpx *.svg *.psd *.cdr *.pcd *.dxf *.ufo *.eps *.ai *.raw *.wmf *.webp);;"
                         "视频文件(*.mp4)";
        QString url = QFileDialog::getOpenFileName(nullptr, "选择文件", "", filter);
        emit sig_appendFile(url);
    }
    return ClickLabel::mousePressEvent(e);
}
