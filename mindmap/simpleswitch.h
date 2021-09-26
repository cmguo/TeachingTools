#ifndef SIMPLESWITCH_H
#define SIMPLESWITCH_H

#include "mindswitch.h"

class SimpleSwitch : public MindSwitch
{
public:
    SimpleSwitch();

    ~SimpleSwitch() override;

public:
    QRectF boundingRect() const override;

    void draw(QPainter *painter, QRectF  const & exposedRect) override;
};

#endif // SIMPLESWITCH_H
