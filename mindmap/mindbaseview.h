#ifndef MINDBASEVIEW_H
#define MINDBASEVIEW_H

#include <QRectF>

class QPainter;
class MindNodeView;

class MindBaseView
{
public:
    virtual ~MindBaseView() {};

    virtual bool isNode() const { return false; }

    virtual QRectF boundingRect() const = 0;

    virtual void draw(QPainter * painter, QRectF  const & exposedRect) = 0;

public:
    void setParent(MindNodeView * parent);

    MindNodeView * parent() const { return parent_; }

    bool hasParent(MindNodeView * parent) const;

protected:
    MindNodeView * parent_ = nullptr;
};

#endif // MINDBASEVIEW_H
