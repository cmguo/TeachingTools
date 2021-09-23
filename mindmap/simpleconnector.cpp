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
    QRectF rc{start_, end_};
    rc = rc.normalized();
    if (!exposedRect.intersects(rc))
        return;
    painter->drawLine(start_, end_);
}
