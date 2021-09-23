#ifndef ARCCONNECTOR_H
#define ARCCONNECTOR_H

#include "mindconnector.h"

class ArcConnector : public MindConnector
{
public:
    ArcConnector();

    ~ArcConnector() override;

public:
    void draw(QPainter *painter, QRectF  const & exposedRect) override;
};

#endif // ARCCONNECTOR_H
