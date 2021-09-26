#include "simpleconnector.h"

#include <QPainter>

SimpleConnector::SimpleConnector()
{
}

SimpleConnector::~SimpleConnector()
{
}

void SimpleConnector::draw(QPainter *painter, QRectF  const & exposedRect)
{
    QPointF s = start();
    QPointF e = end();
    QRectF rc{s, e};
    rc = rc.normalized();
    if (!exposedRect.intersects(rc))
        return;
    painter->drawLine(s, e);
}
