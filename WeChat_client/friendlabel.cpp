#include "friendlabel.h"
#include "ui_friendlabel.h"
#include <QMouseEvent>

FriendLabel::FriendLabel(QWidget *parent):
    QFrame(parent)
    , ui(new Ui::FriendLabel)
{
    ui->setupUi(this);

    ui->close_lb->installEventFilter(this);
}

FriendLabel::~FriendLabel()
{
    delete ui;
}

void FriendLabel::setText(const QString &text)
{
    ui->lb->setText(text);
    ui->lb->setAlignment(Qt::AlignCenter);
}

QString FriendLabel::text() const
{
    return ui->lb->text();
}

bool FriendLabel::eventFilter(QObject *o, QEvent *e)
{
    if(o == ui->close_lb && e->type() == QEvent::MouseButtonPress){
        emit close(ui->lb->text());
    }

    return QFrame::eventFilter(o, e);
}