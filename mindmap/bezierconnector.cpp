#include "bezierconnector.h"
#include "mindnodeview.h"
#include "mindviewstyle.h"

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
    QPointF s = start();
    QPointF e = end();
    QRectF rc{s, e};
    rc = rc.normalized();
    if (!exposedRect.intersects(rc.adjusted(-1, -1, 1, 1)))
        return;
    painter->setPen(QPen(parent()->style()->connectColor(), 2));
    painter->setBrush(QBrush());
    if (qAbs(s.y() - e.y()) < 1) {
        painter->drawLine(s, e);
    } else {
        qreal w = e.x() - s.x();
        QPointF c1{s.x() + w / 2, s.y()};
        QPointF c2{e.x() - w / 2, e.y()};
        QPainterPath path;
        path.moveTo(s);
        path.cubicTo(c1, c2, e);
        painter->drawPath(path);
    }
}
