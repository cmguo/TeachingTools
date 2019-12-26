#include "writinggridcontrol.h"
#include "writinggrid.h"

#include <QBrush>
#include <QEvent>
#include <QGraphicsSceneResizeEvent>
#include <views/itemframe.h>
#include <QPen>
#include <QDebug>
#include <core/resourceview.h>
#include <QGraphicsProxyWidget>
#include <QUrl>
#include <Windows/Controls/inkcanvas.h>

static char const * toolstr =
        "changeGridType()|转换|:/showboard/icons/icon_delete.png;";

WritingGridControl::WritingGridControl(ResourceView * res)
    : Control(res, {KeepAspectRatio})
{
}

QGraphicsItem *WritingGridControl::create(ResourceView *res)
{   QString path = res->url().path();
    int type = res->url().path().split("/")[1].toInt(0);
    QGraphicsItem *item =new WritingGrid(300,WritingGridType(type));
    return item;
}

void WritingGridControl::attaching()
{
    ItemFrame *frame = itemFrame();
    frame->setBrush(QBrush(Qt::white));
    frame->addTopBar();
    frame->addDockItem(ItemFrame::Top, 22);
    frame->addDockItem(ItemFrame::Left, 35);
    frame->addDockItem(ItemFrame::Buttom, 22);
    QGraphicsItem *item = static_cast<WritingGrid*>(item_)->createControlBar();
    frame->addDockItem(ItemFrame::Right,item);
}

void WritingGridControl::attached()
{
    loadFinished(true);
}

QString WritingGridControl::toolsString(QString const & parent) const
{
    return nullptr;
}

void WritingGridControl::changeGridType()
{
    WritingGrid *item = static_cast<WritingGrid*>(item_);
    switch (item->getType()) {
    case WritingGridType::TinWordFormat:
        item->setType(WritingGridType::FourLinesAndThreeGrids);
        break;
    case WritingGridType::FourLinesAndThreeGrids:
        item->setType(WritingGridType::PinYinTinGrids);
        break;
    case WritingGridType::PinYinTinGrids:
        item->setType(WritingGridType::TinWordFormat);
        break;
    }
    sizeChanged();
}


