#include "BezierConnector.h"

#include <QPainter>
#include <QPainterPath>

BezierConnector::BezierConnector()
{
}

BezierConnector::~BezierConnector()
{
}

void BezierConnector::draw(QPainter *painter, QRectF  const & exposedRect)
{
    QRectF rc{start_, end_};
    rc = rc.normalized();
    if (!exposedRect.intersects(rc.adjusted(-1, -1, 1, 1)))
        return;
    if (qAbs(start_.y() - end_.y()) < 1) {
        painter->drawLine(start_, end_);
    } else {
        qreal w = end_.x() - start_.x();
        QPointF c1{start_.x() + w / 4, start_.y()};
        QPointF c2{end_.x() - w / 4, end_.y()};
        QPainterPath path;
        path.moveTo(start_);
        path.cubicTo(c1, c2, end_);
        painter->drawPath(path);
    }
}
