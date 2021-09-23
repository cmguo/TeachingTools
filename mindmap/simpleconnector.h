#ifndef SIMPLECONNECTOR_H
#define SIMPLECONNECTOR_H

#include "mindconnector.h"

class SimpleConnector : public MindConnector
{
public:
    SimpleConnector();

    ~SimpleConnector() override;

public:
    void draw(QPainter *painter, QRectF  const & exposedRect) override;
};

#endif // SIMPLECONNECTOR_H
