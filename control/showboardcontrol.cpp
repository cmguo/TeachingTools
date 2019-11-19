#include "showboardcontrol.h"

#include <whitinggrid.h>

static char const * toolstr =
        "decGrid()|减少|:/showboard/icons/icon_delete.png;"
        "changeGridType()|转换|:/showboard/icons/icon_delete.png;"
        "addGrid()|增加|:/showboard/icons/icon_refresh.png;";
ShowBoardControl::ShowBoardControl(ResourceView * res)
    : Control(res, {KeepAspectRatio})
{
}

QGraphicsItem *ShowBoardControl::create(ResourceView *res)
{

    QGraphicsItem *item =new WhitingGrid(300,WhitingGridType::TinWordFormat);

    return item;
}

QString ShowBoardControl::toolsString(QString const & parent) const
{
    return toolstr;
}

void ShowBoardControl::addGrid()
{
   static_cast<WhitingGrid*>(item_)->addGrid();
    sizeChanged();
}

void ShowBoardControl::decGrid()
{
    static_cast<WhitingGrid*>(item_)->decGrid();
    sizeChanged();
}

void ShowBoardControl::changeGridType()
{
    WhitingGrid *item = static_cast<WhitingGrid*>(item_);
    switch (item->getType()) {
    case WhitingGridType::TinWordFormat:
        item->setType(WhitingGridType::FourLinesAndThreeGrids);
        break;
    case WhitingGridType::FourLinesAndThreeGrids:
        item->setType(WhitingGridType::PinYinTinGrids);
        break;
    case WhitingGridType::PinYinTinGrids:
        item->setType(WhitingGridType::TinWordFormat);
        break;
    }
    sizeChanged();
}


