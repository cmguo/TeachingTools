#ifndef MINDCONNECTOR_H
#define MINDCONNECTOR_H

#include "mindbaseview.h"

#include <QPointF>

class MindConnector : public MindBaseView
{
public:
    virtual ~MindConnector() {}

public:
    QRectF boundingRect() const override;

public:
    void setTarget(MindNodeView * target);

protected:
    QPointF start() const;

    QPointF end() const;

private:
    MindNodeView * target_ = nullptr;
};

#endif // MINDCONNECTOR_H
