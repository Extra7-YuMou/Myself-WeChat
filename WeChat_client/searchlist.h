#ifndef SEARCHLIST_H
#define SEARCHLIST_H

#include <QListWidget>
#include "UserData.h"
#include "findresultdialog.h"
#include <functional>

class SearchList : public QListWidget
{
    Q_OBJECT
    using handler = std::function<void(std::shared_ptr<SearchInfo>)>;
public:
    SearchList(QWidget* parent = nullptr);
    void setSearchEdit(QWidget* search_edit);
protected:
    bool eventFilter(QObject* o, QEvent* e);
private:
    void addTipItem();
    void clearFindDialog();
    void initHandlers();

    std::shared_ptr<FindResultDialog> _find_dlg;
    QWidget* _search_edit;
    QMap<ReqId, handler> _handlers;
private slots:
    void slot_item_clicked(QListWidgetItem* item);
    void slot_search_user_success(std::shared_ptr<SearchInfo>);
};

#endif // SEARCHLIST_H
