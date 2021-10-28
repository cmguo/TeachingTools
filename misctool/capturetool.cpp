#include "capturetool.h"

#include <core/resourceview.h>

#include <QGraphicsRectItem>
#include <QGraphicsScene>
#include <QPen>

CaptureTool::CaptureTool(ResourceView * res)
    : Control(res, {LayoutScale, FullSelect, FixedOnCanvas}, {CanRotate})
{
    res->setProperty("name", "屏幕截图");
}

class CaptureItem : public QGraphicsRectItem
{
public:
    CaptureItem()
    {
        setRect({0, 0, 400, 300});
        setFlag(ItemHasNoContents, true);
    }
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *) override
    {
        QRectF rect = mapToScene(boundingRect()).boundingRect();
        scene()->render(painter, boundingRect(), rect);
    }
};

ControlView *CaptureTool::create(ControlView *parent)
{
    (void) parent;
    return new CaptureItem;
}

void CaptureTool::resize(const QSizeF &size)
{
    static_cast<QGraphicsRectItem*>(item_)->setRect({QPointF(), size});
    Control::resize(size);
}

