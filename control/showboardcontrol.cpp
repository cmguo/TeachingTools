#include "showboardcontrol.h"

#include <QBrush>
#include <QEvent>
#include <QGraphicsSceneResizeEvent>
#include <whitinggrid.h>
#include <views/itemframe.h>
#include <QPen>
#include <QDebug>

static char const * toolstr =
        "changeGridType()|转换|:/showboard/icons/icon_delete.png;";
ShowBoardControl::ShowBoardControl(ResourceView * res)
    : Control(res, {KeepAspectRatio})
{
}

QGraphicsItem *ShowBoardControl::create(ResourceView *res)
{

    QGraphicsItem *item =new WhitingGrid(300,WhitingGridType::TinWordFormat);
    return item;
}

void ShowBoardControl::attaching()
{
    ItemFrame *frame = itemFrame();
    frame->setBrush(QBrush(Qt::white));
    frame->addDockItem(ItemFrame::Top, 22);
    frame->addDockItem(ItemFrame::Left, 35);
    frame->addDockItem(ItemFrame::Buttom, 22);
    QGraphicsItem *item = static_cast<WhitingGrid*>(item_)->createControlBar();
    frame->addDockItem(ItemFrame::Right,item);
}

void ShowBoardControl::attached()
{
    loadFinished(true);
}

QString ShowBoardControl::toolsString(QString const & parent) const
{
    return toolstr;
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


