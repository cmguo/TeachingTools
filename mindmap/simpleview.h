#ifndef SIMPLEVIEW_H
#define SIMPLEVIEW_H

#include "mindnodeview.h"

class SimpleView : public MindNodeView
{
public:
    SimpleView(MindNode * node);

public:
    QSizeF size() override;

    QPointF inPort() override;

    QPointF outPort() override;

    void draw(QPainter *painter, QRectF  const & exposedRect) override;

private:
    QSizeF size_;
};

#endif // SIMPLEVIEW_H
