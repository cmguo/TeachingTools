#include "mindspacing.h"
#include "mindnodeview.h"

#include <QPainter>

QRectF MindSpacing::boundingRect() const
{
    return bounding_;
}

void MindSpacing::draw(QPainter *painter, const QRectF &)
{
    QRectF rc = boundingRect();
    rc.adjust(2, 2, -2, -2);
    painter->setPen(QPen(Qt::yellow, 4));
    painter->setBrush(QBrush());
    painter->drawRect(rc);
}

void MindSpacing::setIgnore(MindNodeView *ignore)
{
    prev_ = ignore;
    next_ = nullptr;
}

bool MindSpacing::setPrevNext(MindNodeView *prev, MindNodeView *next)
{
    if (prev == prev_ || next == prev_)
        return false;
    prev_ = prev;
    next_ = next;
    bounding_ = {QPointF{prev_->pos().x(), prev_->pos().y() + prev_->size().height()},
                QPointF{next_->pos().x() + prev_->size().width(), next_->pos().y()}};
    return true;
}

void MindSpacing::saveLast(const QRectF &bounding)
{
    bounding_ = bounding;
}
