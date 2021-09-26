#include "mindconnector.h"
#include "mindnodeview.h"
#include "mindnode.h"
#include "mindviewtemplate.h"
#include "mindviewstyle.h"

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
    pos2_ = pos_;
    if (size_.isEmpty())
        size_ = style_->measureNode(node_);
    if (!node_->expanded_ || node_->children_.empty()) {
        tl.xoffset += size_.width();
        tl.yoffset += size_.height();
        tl.pop(pos_);
        return;
    }
    if (children_.empty()) {
        for (auto & c : node_->children_) {
            children_.append({tl.createView(&c), tl.createConnector()});
            children_.back().first->setParent(this);
        }
    }
    tl.xoffset += size_.width() + tl.levelPadding;
    for (auto & c : children_) {
        if (c.first == nullptr) {
            c.first = tl.createView(reinterpret_cast<MindNode*>(c.second));
            c.first->setParent(this);
            c.second = tl.createConnector();
        }
        c.first->layout(tl);
        tl.yoffset += tl.siblinPadding;
    }
    tl.yoffset -= tl.siblinPadding;
    pos_.setY((pos_.y() + tl.yoffset - size_.height()) / 2);
    for (auto & c : children_) {
        c.second->setEndian(pos_ + style_->outPort(size_), c.first->pos_ + c.first->style_->inPort(c.first->size_));
    }
    tl.pop(pos_);
}

void MindNodeView::collectShape(QPainterPath &shape)
{
    shape.addRect({pos_, size_});
    if (!node_->expanded_ || node_->children_.empty()) {
        return;
    }
    for (auto & c : children_) {
        c.first->collectShape(shape);
    }
}

MindNodeView *MindNodeView::hitTest(const QPointF &point, MindNodeView * middle)
{
    if (QRectF{pos_, size_}.contains(point))
        return this;
    if (middle && point.x() < pos_.x() + size_.width()) {
        MindNodeView * siblin = nullptr;
        if (this != middle->parent() && parent_ && (siblin = parent_->findChild(this)) && siblin != middle->parent()) {
            middle->pos_ = pos_ + QPointF{0, size_.height()};
            middle->size_ = {size_.width(), siblin->pos_.y() - middle->pos_.y()};
            middle->parent_ = this;
            return middle;
        }
        return nullptr;
    }
    if (!node_->expanded_ || node_->children_.empty()) {
        return nullptr;
    }
    qreal x = children_.front().first->pos_.x();
    if (point.x() < x) {
        return nullptr;
    }
    MindNodeView * last = nullptr;
    for (auto & c : children_) {
        if (c.first->pos2_.y() > point.y()) {
            break;
        }
        last = c.first;
    }
    return last == nullptr ? nullptr : last->hitTest(point, middle);
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

void MindNodeView::setViewStyle(const MindViewStyle *style)
{
    style_ = style;
}

void MindNodeView::setParent(MindNodeView *parent)
{
    parent_ = parent;
}

bool MindNodeView::hasParent(MindNodeView * parent) const
{
    MindNodeView * p = parent_;
    while (p && p != parent) {
        p = p->parent_;
    }
    return p == parent;
}

bool MindNodeView::toggle()
{
    node_->expanded_ = !node_->expanded_;
    return node_->expanded_;
}

void MindNodeView::insertChild(MindNode const & node, MindNodeView *after)
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
    node_->children_.insert(n, node);
    node_->expanded_ = true;
    if (!children_.empty())
        children_.insert(n, {nullptr, reinterpret_cast<MindConnector*>(&node_->children_[n])});
}

void MindNodeView::removeFromParent()
{
    if (parent_ == nullptr)
        return;
    parent_->removeChild(this);

}

void MindNodeView::moveToParent(MindNodeView *toParent, MindNodeView *after)
{
    if (parent_ == nullptr)
        return;
    parent_->moveChild(this, toParent, after);
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

MindNodeView *MindNodeView::findChild(MindNodeView *after)
{
    int n = node_->children_.size() - 1;
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
    return children_.at(n).first;
}

void MindNodeView::moveChild(MindNodeView *child, MindNodeView *toParent, MindNodeView *after)
{
    int m = 0;
    for (auto c : children_) {
        if (c.first == child) {
            break;
        }
        ++m;
    }
    auto cld = children_.at(m);
    MindNode node = node_->children_.at(m);
    children_.removeAt(m);
    node_->children_.removeAt(m);
    if (toParent == this) {
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
        children_.insert(n, cld);
        node_->children_.insert(n, node);
        cld.first->node_ = &node_->children_[n];
    } else {
        delete cld.first;
        delete cld.second;
        toParent->insertChild(node, after);
    }
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
