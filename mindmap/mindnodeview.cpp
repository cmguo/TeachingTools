#include "mindconnector.h"
#include "mindswitch.h"
#include "mindnodeview.h"
#include "mindnode.h"
#include "mindviewtemplate.h"
#include "mindviewstyle.h"
#include "mindspacing.h"

#include <QPainterPath>

MindNodeView::MindNodeView(MindNode * node)
{
    node_ = node;
}

MindNodeView::~MindNodeView()
{
    if (switch_)
        delete switch_;
    for (auto & c : children_) {
        delete c.first;
        delete c.second;
    }
}

void MindNodeView::layout(MindViewTemplate & tl)
{
    tl.push(pos_);
    if (size_.isEmpty())
        size_ = style_->measureNode(node_);
    rect_.setTopLeft({pos_.x() + size_.width(), pos_.y()});
    if (switch_ == nullptr && !node_->children_.empty()) {
        switch_ = tl.createSwitch();
        switch_->setParent(this);
    }
    if (!node_->expanded_ || node_->children_.empty()) {
        tl.xoffset += size_.width();
        tl.yoffset += size_.height();
        if (switch_) {
            tl.xoffset = qMax(tl.xoffset, switch_->boundingRect().right());
        }
        tl.pop(pos_);
        return;
    }
    if (children_.empty()) {
        for (auto & n : node_->children_) {
            children_.append({tl.createView(&n), tl.createConnector()});
            auto & c = children_.back();
            c.first->setParent(this);
            c.second->setParent(this);
            c.second->setTarget(c.first);
        }
    }
    tl.xoffset += size_.width() + tl.levelPadding;
    for (auto & c : children_) {
        if (c.first == nullptr) {
            c.first = tl.createView(reinterpret_cast<MindNode*>(c.second));
            c.first->setParent(this);
            c.second = tl.createConnector();
            c.second->setParent(this);
            c.second->setTarget(c.first);
        }
        c.first->layout(tl);
        tl.yoffset += tl.siblinPadding;
    }
    tl.yoffset -= tl.siblinPadding;
    rect_.setBottomRight({tl.xmax, tl.yoffset});
    int oldy = pos_.y();
    pos_.setY((pos_.y() + tl.yoffset - size_.height()) / 2);
    if (oldy > pos_.y()) {
        moveBy({0, oldy - pos_.y()});
        tl.yoffset = pos_.y() + size_.height();
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

QRectF MindNodeView::boundingRect() const
{
    return {pos_, size_};
}

MindSpacing MindNodeView::HitTestSpacing;

MindBaseView *MindNodeView::hitTest(const QPointF &point, int types)
{
    if (QRectF{pos_, size_}.contains(point))
        return (types & NodeOnly) ? this : nullptr;
    if ((types & 4) && switch_ && switch_->boundingRect().contains(point))
        return switch_;
    if ((types & 2) && point.x() < pos_.x() + size_.width() && this != HitTestSpacing.prev() && parent_) {
        if (point.y() < pos_.y()) {
            MindNodeView * siblin = parent_->findChildBefore(this);
            if (HitTestSpacing.setPrevNext(siblin, this))
                return &HitTestSpacing;
        } else {
            MindNodeView * siblin = parent_->findChildAfter(this);
            if (HitTestSpacing.setPrevNext(this, siblin))
                return &HitTestSpacing;
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
        if (c.first->rect_.top() > point.y()) {
            break;
        }
        last = c.first;
    }
    return last == nullptr ? nullptr : last->hitTest(point, types);
}

void MindNodeView::draw(QPainter *painter, QRectF const & exposedRect)
{
    if (!node_->expanded_ || node_->children_.empty()) {
        if (switch_)
            switch_->draw(painter, exposedRect);
        return;
    }
    for (auto & c : children_) {
        c.second->draw(painter, exposedRect);
        c.first->draw(painter, exposedRect);
    }
    if (switch_)
        switch_->draw(painter, exposedRect);
}

void MindNodeView::setStyle(const MindViewStyle *style)
{
    style_ = style;
}

QPointF MindNodeView::inPort() const
{
    return pos_ + style_->inPort(size_);
}

QPointF MindNodeView::outPort() const
{
    return pos_ + style_->outPort(size_);
}

QPointF MindNodeView::switchPort() const
{
    return pos_ + style_->switchPort(size_);
}

void MindNodeView::setTitle(const QString &title)
{
    node_->title = title;
    size_ = {0, 0};
}

bool MindNodeView::toggle()
{
    node_->expanded_ = !node_->expanded_;
    return node_->expanded_;
}

static MindNodeView * FIRST_NODE = reinterpret_cast<MindNodeView*>(1);
static MindNodeView * LAST_NODE = reinterpret_cast<MindNodeView*>(2);

int MindNodeView::insertChild(MindNode const & node, MindNodeView *after)
{
    int n = findChild(after, false);
    node_->children_.insert(n, node);
    node_->expanded_ = true;
    if (!children_.empty())
        children_.insert(n, {nullptr, reinterpret_cast<MindConnector*>(&node_->children_[n])});
    return n;
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

MindNodeView *MindNodeView::childAt(int index)
{
    return children_.at(index).first;
}

MindNodeView *MindNodeView::findChildBefore(MindNodeView *before)
{
    int n = findChild(before, true);
    return (n >= 0 && n < children_.size()) ? children_.at(n).first : nullptr;
}

MindNodeView *MindNodeView::findChildAfter(MindNodeView *after)
{
    int n = findChild(after, false);
    return (n >= 0 && n < children_.size()) ? children_.at(n).first : nullptr;
}

int MindNodeView::findChild(MindNodeView *node, bool beforeOrAfter)
{
    if (node == nullptr)
        return beforeOrAfter ? 0 : node_->children_.size();
    if (node == this)
        return beforeOrAfter ? node_->children_.size() : 0;
    int n = 0;
    for (auto c : children_) {
        if (c.first == node) {
            if (beforeOrAfter)
                --n;
            else
                ++n;
            break;
        }
        ++n;
    }
    return n;
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
        int n = findChild(after, false);
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

void MindNodeView::moveBy(const QPointF &off)
{
    pos_ += off;
    if (node_->expanded_) {
        for (auto & c : children_)
            c.first->moveBy(off);
    }
}
