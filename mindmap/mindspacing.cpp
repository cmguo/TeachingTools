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
    parent_ = nullptr;
    next_ = nullptr;
}

bool MindSpacing::setPrevNext(MindNodeView *prev, MindNodeView *next)
{
    if (prev == prev_ || next == prev_)
        return false;
    if (prev == nullptr) {
        prev_ = parent_ = next->parent();
        next_ = next;
        bounding_ = {QPointF{next->pos().x(), parent_->rect().top()},
                    QPointF{next->pos().x() + next->size().width(), next->pos().y()}};
    } else if (next == nullptr) {
        prev_ = prev;
        next_ = parent_ = prev->parent();
        bounding_ = {QPointF{prev->pos().x(), prev->pos().y() + prev->size().height()},
                    QPointF{prev->pos().x() + prev->size().width(), parent_->rect().bottom()}};
    } else {
        prev_ = prev;
        parent_ = next->parent();
        next_ = next;
        bounding_ = {QPointF{prev->pos().x(), prev->pos().y() + prev->size().height()},
                    QPointF{next->pos().x() + prev->size().width(), next->pos().y()}};
    }
    return true;
}

void MindSpacing::saveLast(const QRectF &bounding)
{
    bounding_ = bounding;
}
