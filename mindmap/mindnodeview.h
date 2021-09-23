#ifndef MINDNODEVIEW_H
#define MINDNODEVIEW_H

#include <QList>
#include <QPainter>
#include <QPointF>
#include <QSizeF>

class MindNode;
class MindViewTemplate;
class MindConnector;

class MindNodeView
{
public:
    MindNodeView(MindNode * node);

    virtual ~MindNodeView();

public:
    virtual QSizeF size() = 0;

    virtual QPointF inPort() = 0;

    virtual QPointF outPort() = 0;

    virtual void layout(MindViewTemplate & tl);

    // call after layout
    virtual void collectShape(QPainterPath & shape);

    // call after layout
    virtual MindNodeView * hitTest(QPointF const & point);

    // call after layout
    virtual void draw(QPainter * painter, QRectF  const & exposedRect);

public:
    void setParent(MindNodeView * parent);

    MindNodeView * parent() const { return parent_; }

    QPointF pos() const { return pos_; }

public:
    void toggle();

    void newChild(MindNodeView * after = nullptr);

    void remove();

    void removeChild(MindNodeView * child);

    enum FocusDirection {
        FocusLeft,
        FocusUp,
        FocusRight,
        FocusDown,
    };

    MindNodeView * nextFocus(FocusDirection dir);

protected:
    MindNodeView * parent_ = nullptr;
    QList<QPair<MindNodeView*, MindConnector*>> children_;
    QPointF pos_;

protected:
    MindNode * node_;
};

#endif // MINDNODEVIEW_H
