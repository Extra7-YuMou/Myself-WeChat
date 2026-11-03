#include "applyfrienddialog.h"
#include "ui_applyfrienddialog.h"
#include "UserMgr.h"
#include "friendlabel.h"
#include "tcpmgr.h"

ApplyFriendDialog::ApplyFriendDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::ApplyFriendDialog)
{
    ui->setupUi(this);

    setWindowFlags(windowFlags() | Qt::FramelessWindowHint);
    this->setModal(true);
    this->setObjectName("ApplyFriendDialog");

    ui->input_wid->hide();
    ui->name_edit->setLimitLength(20);
    ui->name_edit->setPlaceholderText("我是" + UserMgr::GetInstance()->getNick());
    ui->back_edit->setLimitLength(20);
    ui->back_edit->setPlaceholderText("添加备注");
    ui->lb_edit->setLimitLength(20);
    ui->lb_edit->setPlaceholderText("搜索/添加标签");

    ui->scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    ui->scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    ui->scrollArea->installEventFilter(this);

    ui->sure_btn->setAutoDefault(false);
    ui->cancel_btn->setAutoDefault(false);

    connect(ui->lb_edit, &CustomizeEdit::returnPressed, this, &ApplyFriendDialog::SlotLabelEnter);
    connect(ui->lb_edit, &CustomizeEdit::textChanged, this, &ApplyFriendDialog::SlotLabelTextChange);
    connect(ui->lb_edit, &CustomizeEdit::editingFinished, this, &ApplyFriendDialog::SlotLabelEditFinished);
    connect(ui->tip_lb, &ClickOnceLabel::clicked, this, &ApplyFriendDialog::SlotAddFirendLabelByClickTip);
    connect(ui->more_lb, &ClickOnceLabel::clicked, this, &ApplyFriendDialog::ShowMoreLabel);

    _tip_cur_point = QPoint(5, 5);
    _tip_data = {"同学","家人","菜鸟教程","C++ Primer","Rust 程序设计",
                 "父与子学Python","nodejs开发指南","go 语言开发指南",
                 "游戏伙伴","金融投资","微信读书","拼多多拼友" };
    initTipLabels();
}

ApplyFriendDialog::~ApplyFriendDialog()
{
    delete ui;
}

bool ApplyFriendDialog::eventFilter(QObject *o, QEvent *e)
{
    if(o == ui->scrollArea && e->type() == QEvent::Enter){
        ui->scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    }else if(o == ui->scrollArea && e->type() == QEvent::Leave){
        ui->scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    }
    return QDialog::eventFilter(o, e);
}

void ApplyFriendDialog::mousePressEvent(QMouseEvent *e)
{
    if(e->type() == QMouseEvent::MouseButtonPress && e->button() == Qt::LeftButton){
        offset = e->globalPos() - this->pos();
    }

    QDialog::mousePressEvent(e);
}

void ApplyFriendDialog::mouseMoveEvent(QMouseEvent *e)
{
    if(e->type() == QMouseEvent::MouseMove){
        QPoint windowPos = e->globalPos() - offset;
        this->move(windowPos);
    }

    QDialog::mouseMoveEvent(e);
}

void ApplyFriendDialog::resetLabel()
{
    auto max_width = ui->grid_wid->width();
    auto label_height = 0;
    for(auto iter = _friend_labels.begin(); iter != _friend_labels.end(); iter++){
        //todo... 添加宽度统计
        if( _label_point.x() + iter.value()->width() > max_width) {
            _label_point.setY(_label_point.y()+iter.value()->height()+6);
            _label_point.setX(2);
        }
        iter.value()->move(_label_point);
        iter.value()->show();
        _label_point.setX(_label_point.x()+iter.value()->width()+2);
        _label_point.setY(_label_point.y());
        label_height = iter.value()->height();
    }
    if(_friend_labels.isEmpty()){
        ui->lb_edit->move(_label_point);
        return;
    }
    // if(_label_point.x() + MIN_APPLY_LABEL_ED_LEN > ui->gridWidget->width()){
    //     ui->lb_edit->move(2,_label_point.y()+label_height+6);
    // }else{
    //     ui->lb_edit->move(_label_point);
    // }
}

void ApplyFriendDialog::addLabel(QString name)
{
    if(_friend_labels.find(name) != _friend_labels.end()){
        return;
    }

    auto iter_find = _add_labels.find(name);
    if(iter_find != _add_labels.end()){
        iter_find.value()->setCurState(ClickState::selected);
        _tip_data.push_back(name);
        return;
    }

    auto* lb = new FriendLabel(ui->lb_list);
    lb->setText(name);
    lb->setObjectName("FriendLabel");

    QFontMetrics fm(lb->text());
    int textW = fm.horizontalAdvance(lb->text());
    int textH = fm.height();
    lb->setMinimumSize(QSize(textW + 15, textH + 3));

    if(lb->width() + _tip_cur_point.x() > ui->lb_list->width()){
        _tip_cur_point.setX(0);
        _tip_cur_point.setY(_tip_cur_point.y() + textH + 10);
    }

    lb->move(_tip_cur_point);
    lb->show();
    _friend_labels.insert(lb->text(), lb);
    _friend_label_keys.push_back(lb->text());

    connect(lb, &FriendLabel::close, this, &ApplyFriendDialog::SlotRemoveFriendLabel);
    _tip_cur_point.setX(_tip_cur_point.x() + lb->width() + 10);
    _tip_cur_point.setY(_tip_cur_point.y());
}

void ApplyFriendDialog::initTipLabels()
{
    int lines = 1;
    for(int i = 0; i < _tip_data.size(); i++){
        auto* _tip_lb = new ClickLabel(ui->lb_list);
        _tip_lb->setState("normal", "hover", "press",
                          "select", "select_hover", "select_press");
        _tip_lb->setObjectName("tipsLb");
        _tip_lb->setText(_tip_data[i]);
        connect(_tip_lb, &ClickLabel::clicked, this, &ApplyFriendDialog::SlotChangeFriendLabelByTip);

        QFontMetrics fm(_tip_lb->font());
        int textW = fm.horizontalAdvance(_tip_lb->text());
        int textH = fm.height();

        if(_tip_cur_point.x() + _tip_lb->width() > ui->lb_list->width()){
            _tip_cur_point.setX(0);
            _tip_cur_point.setY(_tip_cur_point.y() + textH + 15);
        }

        auto next_point = _tip_cur_point;
        _tip_lb->setMinimumSize(QSize(textW + 10, textH + 4));
        _tip_lb->setAlignment(Qt::AlignCenter);
        addTipLabel(_tip_lb, _tip_cur_point, next_point, textW, textH);
        _tip_cur_point = next_point;
    }
}

void ApplyFriendDialog::addTipLabel(ClickLabel * lb, QPoint _cur_point, QPoint & _next_point, int textW, int textH)
{
    lb->move(_cur_point);
    lb->show();
    _add_labels.insert(lb->text(), lb);
    _add_label_keys.push_back(lb->text());
    _next_point = QPoint(_cur_point.x() + lb->width() + 10, _cur_point.y());
}

void ApplyFriendDialog::setSearchInfo(std::shared_ptr<SearchInfo> si)
{
    _si = si;
    ui->back_edit->setText(_si->_nick);
}

void ApplyFriendDialog::ShowMoreLabel()
{
    ui->more_lb_wid->hide();

    _tip_cur_point = QPoint(0, 5);
    auto next_point = _tip_cur_point;
    int textW = 0;
    int textH = 0;

    for(auto& add_key : _add_label_keys){
        auto* add_lb = _add_labels[add_key];

        QFontMetrics fm(add_lb->text());
        textW = fm.horizontalAdvance(add_lb->text());
        textH = fm.height();

        if(textW + _tip_cur_point.x() > ui->lb_list->width()){
            _tip_cur_point.setX(0);
            _tip_cur_point.setY(_tip_cur_point.y() + textH + 10);
        }

        add_lb->move(_tip_cur_point);
        next_point = QPoint(_tip_cur_point.x() + add_lb->width() + 10, _tip_cur_point.y());
        _tip_cur_point = next_point;
    }


    //添加未添加的
    for(int i = 0; i < _tip_data.size(); i++){
        auto iter = _add_labels.find(_tip_data[i]);
        if(iter != _add_labels.end()){
            continue;
        }
        auto* lb = new ClickLabel(ui->lb_list);
        lb->setState("normal", "normal_hover", "normal_pressed", "select_normal",
                     "select_hover", "select_pressed");
        lb->setObjectName("tipslb");
        lb->setText(_tip_data[i]);
        connect(lb, &ClickLabel::clicked, this, &ApplyFriendDialog::SlotChangeFriendLabelByTip);
        QFontMetrics fontMetrics(lb->text()); // 获取QLabel控件的字体信息
        int textWidth = fontMetrics.horizontalAdvance(lb->text()); // 获取文本的宽度
        int textHeight = fontMetrics.height(); // 获取文本的高度
        if (_tip_cur_point.x() + lb->width() > ui->lb_list->width()) {
            _tip_cur_point.setX(0);
            _tip_cur_point.setY(_tip_cur_point.y() + textHeight + 10);
        }
        next_point = _tip_cur_point;
        addTipLabel(lb, _tip_cur_point, next_point, textWidth, textHeight);
        _tip_cur_point = next_point;
    }
}

void ApplyFriendDialog::SlotLabelEnter()
{
    if(ui->lb_edit->text().isEmpty()){
        return;
    }

    auto text = ui->lb_edit->text();

    auto iter_find = std::find(_tip_data.begin(), _tip_data.end(), text);
    if(iter_find == _tip_data.end()){
        _tip_data.push_back(text);
    }

    auto iter_add = _add_labels.find(text);
    if(iter_add != _add_labels.end()){
        iter_add.value()->setCurState(ClickState::selected);
        return;
    }

    auto iter_friend = std::find(_friend_label_keys.begin(), _friend_label_keys.end(), text);
    if(iter_friend != _friend_label_keys.end()){
        return;
    }

    addLabel(text);
}

void ApplyFriendDialog::SlotRemoveFriendLabel(QString text)
{
    auto find_it = _friend_labels.find(text);
    if(find_it == _friend_labels.end()){
        return;
    }

    auto iter_find = _friend_label_keys.end();
    for(auto it = _friend_label_keys.begin(); it != _friend_label_keys.end(); it++){
        if(*it == text){
            iter_find = it;
            break;
        }
    }

    if(iter_find == _friend_label_keys.end()){
        return;
    }

    delete find_it.value();
    _friend_labels.erase(find_it);
    auto iter_key = std::find(_friend_label_keys.begin(), _friend_label_keys.end(), text);
    _friend_label_keys.erase(iter_key);
    resetLabel();
}

void ApplyFriendDialog::SlotChangeFriendLabelByTip(QString name, ClickState state)
{
    auto iter_lb = _add_labels.find(name);
    if(iter_lb == _add_labels.end()){
        return;
    }

    if(state == ClickState::selected){
        //编写添加逻辑
        addLabel(name);
        return;
    }
    if(state == ClickState::normal){
        //编写删除逻辑
        SlotRemoveFriendLabel(name);
        return;
    }
}

void ApplyFriendDialog::SlotLabelTextChange(const QString &text)
{
    if(text.isEmpty()){
        return;
    }

    auto iter_add = std::find(_tip_data.begin(), _tip_data.end(), text);
    auto iter_friend = std::find(_friend_label_keys.begin(), _friend_label_keys.end(), text);
    if(iter_add == _tip_data.end() && iter_friend == _friend_label_keys.end()){
        auto new_tip = "创建新标签:" + text;
        ui->tip_lb->setText(new_tip);
        ui->input_wid->show();
        return;
    }

    ui->tip_lb->setText(text);
    ui->input_wid->show();
}

void ApplyFriendDialog::SlotLabelEditFinished()
{
    ui->input_wid->hide();
}

void ApplyFriendDialog::SlotAddFirendLabelByClickTip(QString text)
{
    if(text.isEmpty()){
        return;
    }

    if(text.contains("创建新标签:")){
        auto new_text = text.mid(QString("创建新标签:").length());
        addLabel(new_text);
        ui->input_wid->hide();
        return;
    }

    for(auto it = _add_labels.begin(); it != _add_labels.end(); it++){
        if(it.key() == text){
            it.value()->setCurState(ClickState::selected);
            return;
        }
    }
}

void ApplyFriendDialog::on_sure_btn_clicked()
{
    for(auto it = _add_labels.begin(); it != _add_labels.end(); it++){
        if(it.value()->GetState() == ClickState::selected){
            _selected_label.push_back(it.key());
        }
    }

    for(auto it = _friend_labels.begin(); it != _friend_labels.end(); it++){
        _selected_label.push_back(it.key());
    }

    for(auto& it : _selected_label){
        qDebug() << it;
    }

    qDebug()<<"Slot Apply Sure called" ;

    auto nameText = ui->name_edit->text();
    if(nameText.isEmpty()){
        nameText = "我是" + UserMgr::GetInstance()->getNick();
    }

    auto backText = ui->back_edit->text();

    QJsonObject json;
    json["from_uid"] = UserMgr::GetInstance()->getUid();
    json["to_uid"] = _si->_uid;
    json["name"] = UserMgr::GetInstance()->getName();
    json["desc"] = nameText;
    json["back"] = backText;
    json["icon"] = UserMgr::GetInstance()->getIcon();
    qDebug() << "json: " << json;
    QJsonDocument doc(json);
    QByteArray data = doc.toJson(QJsonDocument::Compact);
    emit TcpMgr::GetInstance()->sig_send_data(ReqId::ID_APPLY_FRIEND_REQ, data);

    this->hide();
    deleteLater();
}


void ApplyFriendDialog::on_cancel_btn_clicked()
{
    qDebug() << "Slot Apply cancel called";
    this->hide();
    deleteLater();
}

