#include "writinggridcontrol.h"
#include "writinggrid.h"

#include <views/itemframe.h>
#include <views/whitecanvas.h>
#include <core/resourcetransform.h>
#include <core/resourceview.h>

#include <QBrush>
#include <QEvent>
#include <QGraphicsSceneResizeEvent>
#include <QPen>
#include <QDebug>
#include <QGraphicsProxyWidget>
#include <QUrl>
#include <Windows/Controls/inkcanvas.h>
#include <QGuiApplication>
#include <QMimeData>
#include <QScreen>

WritingGridControl::WritingGridControl(ResourceView * res)
#ifdef QT_DEBUG
    : Control(res, {KeepAspectRatio, FixedOnCanvas}, {})
#else
    : Control(res, {KeepAspectRatio, FixedOnCanvas}, {CanRotate})
#endif
{
    int screen_height = QGuiApplication::primaryScreen()-> availableGeometry().height();
	m_adapterRatio = screen_height / 1080.0f;
    setMinSize({0, 232});
}

QGraphicsItem *WritingGridControl::create(ResourceView *res)
{   QString path = res->url().path();
    int type = res->url().path().split("/")[1].toInt();
    QGraphicsItem *item =new WritingGrid(300 * m_adapterRatio,WritingGridType(type));
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
    topItem->setRect(0,0,frame->boundingRect().width(),40 * m_adapterRatio);
    QSize pixItemSize = QSize(40 * m_adapterRatio,40 * m_adapterRatio);
    QGraphicsPixmapItem* pixItem = new QGraphicsPixmapItem(QPixmap(":/teachingtools/icon/icon_drag.png").scaled(pixItemSize, Qt::KeepAspectRatio, Qt::SmoothTransformation),topItem);
    pixItem->setX(topItem->boundingRect().width()/2-pixItemSize.width()/2);
    pixItem->setY(topItem->boundingRect().height()/2-pixItemSize.height()/2);
    frame->addDockItem(ItemFrame::Top, topItem);
    frame->addDockItem(ItemFrame::Left, 72 * m_adapterRatio);
    frame->addDockItem(ItemFrame::Buttom, 16 * m_adapterRatio);
    QGraphicsItem *item = static_cast<WritingGrid*>(item_)->createControlBar();
    frame->addDockItem(ItemFrame::Right,item);
}

void WritingGridControl::attached()
{
    connect(&resource()->transform(), &ResourceTransform::changed,
            this, [this] () {
        WritingGrid *item = static_cast<WritingGrid*>(item_);
        item->inkCanvas()->itemChange(QGraphicsItem::ItemTransformHasChanged, QVariant());
    });
    if (!(flags_ & RestoreSession))
        whiteCanvas()->topControl()->setProperty("editingMode", 0);
    loadFinished(true);
}

void WritingGridControl::afterClone(Control * control)
{
    QSharedPointer<StrokeCollection> strokes(new StrokeCollection);
    qobject_cast<WritingGridControl*>(control)->item_
            ->toGraphicsObject()->setProperty("strokes", QVariant::fromValue(strokes));
}

void WritingGridControl::copy(QMimeData &data)
{
    Control::copy(data);
    data.findChild<ResourceView*>()->setProperty("strokes", QVariant());
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


