#include "mindconnector.h"
#include "mindnodeview.h"
#include "mindnode.h"
#include "mindviewtemplate.h"

#include <QPainterPath>

MindNodeView::MindNodeView(MindNode * node)
{
    node_ = node;
}

MindNodeView::~MindNodeView()
{
    for (auto & c : children_) {
        delete c.first;
        delete c.second;
    }
}

void MindNodeView::layout(MindViewTemplate & tl)
{
    tl.push(pos_);
    pos_ = {tl.xoffset, tl.yoffset};
    if (!node_->expanded_ || node_->children_.empty()) {
        tl.xoffset += size().width();
        tl.yoffset += size().height();
        tl.pop(pos_);
        return;
    }
    if (children_.empty()) {
        for (auto & c : node_->children_) {
            children_.append({tl.createView(&c), tl.createConnector()});
            children_.back().first->setParent(this);
        }
    }
    tl.xoffset += size().width() + tl.levelPadding;
    for (auto & c : children_) {
        if (c.first == nullptr) {
            c.first = tl.createView(reinterpret_cast<MindNode*>(c.second));
            c.second = tl.createConnector();
        }
        c.first->layout(tl);
        tl.yoffset += tl.siblinPadding;
    }
    tl.yoffset -= tl.siblinPadding;
    pos_.setY((pos_.y() + tl.yoffset - size().height()) / 2);
    for (auto & c : children_) {
        c.second->setEndian(pos_ + outPort(), c.first->pos_ + c.first->inPort());
    }
    tl.pop(pos_);
}

void MindNodeView::collectShape(QPainterPath &shape)
{
    shape.addRect({pos_, size()});
    if (!node_->expanded_ || node_->children_.empty()) {
        return;
    }
    for (auto & c : children_) {
        c.first->collectShape(shape);
    }
}

MindNodeView *MindNodeView::hitTest(const QPointF &point)
{
    if (QRectF{pos_, size()}.contains(point))
        return this;
    if (!node_->expanded_ || node_->children_.empty()) {
        return nullptr;
    }
    qreal x = children_.front().first->pos_.x();
    if (x > point.x())
        return nullptr;
    MindNodeView * last = nullptr;
    for (auto & c : children_) {
        if (c.first->pos_.y() > point.y()) {
            break;
        }
        last = c.first;
    }
    return last == nullptr ? nullptr : last->hitTest(point);
}

void MindNodeView::draw(QPainter *painter, QRectF const & exposedRect)
{
    if (!node_->expanded_ || node_->children_.empty()) {
        return;
    }
    for (auto & c : children_) {
        c.second->draw(painter, exposedRect);
        c.first->draw(painter, exposedRect);
    }
}

void MindNodeView::setParent(MindNodeView *parent)
{
    parent_ = parent;
}

void MindNodeView::toggle()
{
    node_->expanded_ = !node_->expanded_;
}

void MindNodeView::newChild(MindNodeView *after)
{
    int n = node_->children_.size();
    if (after) {
        n = 0;
        for (auto c : children_) {
            if (c.first == after) {
                ++n;
                break;
            }
            ++n;
        }
    }
    node_->children_.insert(n, MindNode());
    node_->expanded_ = true;
    if (!children_.empty())
        children_.insert(n, {nullptr, reinterpret_cast<MindConnector*>(&node_->children_[n])});
}

void MindNodeView::remove()
{
    if (parent_ == nullptr)
        return;
    parent_->removeChild(this);

}

void MindNodeView::removeChild(MindNodeView *child)
{
    int n = 0;
    for (auto c : children_) {
        if (c.first == child) {
            delete c.second;
            break;
        }
        ++n;
    }
    children_.removeAt(n);
    delete child;
    node_->children_.removeAt(n);
}

MindNodeView *MindNodeView::nextFocus(MindNodeView::FocusDirection dir)
{
    MindNodeView * next = nullptr;
    switch (dir) {
    case FocusLeft:
        next = parent_;
        break;
    case FocusUp: {
        if (parent_) {
            for (auto c : parent_->children_) {
                if (c.first == this)
                    break;
                next = c.first;
            }
        }
        if (next == nullptr)
            next = parent_;
        break;
    }
    case FocusDown: {
        if (parent_) {
            bool found = false;
            for (auto c : parent_->children_) {
                if (found) {
                    next = c.first;
                    break;
                }
                if (c.first == this)
                    found = true;
            }
        }
        if (next == nullptr && node_->expanded_ && !children_.isEmpty()) {
            next = children_.back().first;
        }
        break;
    }
    case FocusRight:
        if (node_->expanded_ && !children_.isEmpty())
            next = children_.front().first;
        break;
    }
    return next;
}
