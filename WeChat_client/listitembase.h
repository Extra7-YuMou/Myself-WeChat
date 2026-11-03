#ifndef LISTITEMBASE_H
#define LISTITEMBASE_H

#include <QListWidgetItem>
#include "global.h"

class ListItemBase : public QWidget
{
    Q_OBJECT
public:
    explicit ListItemBase(QWidget* parent = nullptr);
    void setItemType(ListItemType itemType);
    ListItemType getItemType();
private:
    ListItemType _itemType;
};

#endif // LISTITEMBASE_H
