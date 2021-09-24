#ifndef SIMPLEVIEW_H
#define SIMPLEVIEW_H

#include "mindnodeview.h"

class SimpleView : public MindNodeView
{
public:
    SimpleView(MindNode * node);

public:
    void draw(QPainter *painter, QRectF  const & exposedRect) override;
};

#endif // SIMPLEVIEW_H
