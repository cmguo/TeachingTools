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
    frame->setBrush(QBrush(0xF4F4F4));
    //frame->addTopBar();
    QGraphicsRectItem *topItem =  new QGraphicsRectItem();
    topItem->setBrush(QBrush(Qt::transparent));
    topItem->setPen(Qt::NoPen);
    topItem->setRect(0,0,frame->boundingRect().width(),40);
    QSize pixItemSize = QSize(40,40);
    QGraphicsPixmapItem* pixItem = new QGraphicsPixmapItem(QPixmap(":/teachingtools/icon/icon_drag.png"),topItem);
    pixItem->setX(topItem->boundingRect().width()/2-pixItemSize.width()/2);
    pixItem->setY(topItem->boundingRect().height()/2-pixItemSize.height()/2);
    frame->addDockItem(ItemFrame::Top, topItem);
    frame->addDockItem(ItemFrame::Left, 72);
    frame->addDockItem(ItemFrame::Buttom, 16);
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


