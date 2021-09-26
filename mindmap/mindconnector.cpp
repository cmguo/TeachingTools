#include "mindconnector.h"
#include "mindnodeview.h"

#include <QLine>

QRectF MindConnector::boundingRect() const
{
    return QRectF{start(), end()}.normalized();
}

void MindConnector::setTarget(MindNodeView *target)
{
    target_ = target;
}

QPointF MindConnector::start() const
{
    return parent_->outPort();
}

QPointF MindConnector::end() const
{
    return target_->inPort();
}


