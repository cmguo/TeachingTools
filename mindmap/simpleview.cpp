#include "simpleview.h"
#include "mindnode.h"
#include "mindviewstyle.h"

SimpleView::SimpleView(MindNode * node)
    : MindNodeView(node)
{
}

void SimpleView::draw(QPainter *painter, QRectF  const & exposedRect)
{
    QRectF rect{pos_, size_};
    if (exposedRect.intersects(rect)) {
        style_->applyTo(painter, rect);
        painter->drawText(rect, Qt::TextDontClip, node_->title);
    }
    MindNodeView::draw(painter, exposedRect);
}
