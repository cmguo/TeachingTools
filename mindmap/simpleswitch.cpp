#include "simpleswitch.h"

#include <QPainter>

SimpleSwitch::SimpleSwitch()
{
}

SimpleSwitch::~SimpleSwitch()
{
}

QRectF SimpleSwitch::boundingRect() const
{
    QPointF pos = this->pos();
    pos.setX(pos.x() + 6);
    QPointF center = {6, 6};
    return {pos - center, pos + center};
}

void SimpleSwitch::draw(QPainter *painter, QRectF  const & exposedRect)
{
    if (expanded() && !hover_)
        return;
    QRectF rc = boundingRect();
    if (!exposedRect.intersects(rc))
        return;
    painter->setPen(QPen(Qt::black, 2));
    painter->setBrush(Qt::white);
    painter->drawEllipse(rc);
    QPointF c = rc.center();
    QPointF off = {3, 0};
    painter->drawLine(c - off, c + off);
    if (!expanded()) {
        off = {0, 3};
        painter->drawLine(c - off, c + off);
    }
}
