#include "arcconnector.h"

#include <QPainter>

ArcConnector::ArcConnector()
{
}

ArcConnector::~ArcConnector()
{
}

void ArcConnector::draw(QPainter *painter, QRectF  const & exposedRect)
{
    QRectF rc{start_, end_};
    rc = rc.normalized();
    if (!exposedRect.intersects(rc))
        return;
    if (rc.height() < 1) {
        painter->drawLine(start_, end_);
    } else if (start_.y() < end_.y()) {
        rc.setTopRight(rc.topRight() + QPointF{rc.width(), -rc.height()});
        painter->drawArc(rc, 180 * 16, 90 * 16);
    } else {
        rc.setSize(rc.size() * 2);
        painter->drawArc(rc, 90 * 16, 90 * 16);
    }
}
