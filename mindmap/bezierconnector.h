#ifndef BEZIERCONNECTOR_H
#define BEZIERCONNECTOR_H

#include "mindconnector.h"

class BezierConnector : public MindConnector
{
public:
    BezierConnector();

    ~BezierConnector() override;

public:
    void draw(QPainter *painter, QRectF  const & exposedRect) override;
};

#endif // BEZIERCONNECTOR_H
