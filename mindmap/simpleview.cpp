#include "simpleview.h"
#include "mindnode.h"

SimpleView::SimpleView(MindNode * node)
    : MindNodeView(node)
{
    size_ = {100, 40};
}

QSizeF SimpleView::size()
{
    return size_;
}

QPointF SimpleView::inPort()
{
    return {0, size_.height() / 2};
}

QPointF SimpleView::outPort()
{
    return {size_.width(), size_.height() / 2};
}

void SimpleView::draw(QPainter *painter, QRectF  const & exposedRect)
{
    QRectF rect{pos_, size_};
    if (exposedRect.intersects(rect)) {
        rect.adjust(5, 5, -5, -5);
        painter->drawRoundedRect(rect, 5, 5);
        rect.adjust(10, 10, -10, -10);
        painter->drawText(rect, node_->title);
    }
    MindNodeView::draw(painter, exposedRect);
}
