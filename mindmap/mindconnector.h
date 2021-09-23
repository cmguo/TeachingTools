#ifndef MINDCONNECTOR_H
#define MINDCONNECTOR_H

#include <QPointF>
#include <QRectF>

class QPainter;

class MindConnector
{
public:
    virtual ~MindConnector() {}

public:
    virtual void setEndian(QPointF const & start, QPointF const &end);

    virtual void draw(QPainter * painter, QRectF  const & exposedRect) = 0;

protected:
    QPointF start_;
    QPointF end_;
};

#endif // MINDCONNECTOR_H
