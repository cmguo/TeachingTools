#ifndef MINDNODEVIEW_H
#define MINDNODEVIEW_H

#include <QList>
#include <QPainter>
#include <QPointF>
#include <QSizeF>

class MindNode;
class MindViewTemplate;
class MindConnector;
class MindViewStyle;

class MindNodeView
{
public:
    MindNodeView(MindNode * node);

    virtual ~MindNodeView();

public:
    virtual void layout(MindViewTemplate & tl);

    // call after layout
    virtual void collectShape(QPainterPath & shape);

    // call after layout
    virtual MindNodeView * hitTest(QPointF const & point, MindNodeView * middle = nullptr);

    // call after layout
    virtual void draw(QPainter * painter, QRectF  const & exposedRect);

public:
    void setViewStyle(MindViewStyle const * style);

    MindViewStyle const * style() const { return style_; }

    void setParent(MindNodeView * parent);

    MindNodeView * parent() const { return parent_; }

    bool hasParent(MindNodeView * parent) const;

    QPointF pos() const { return pos_; }

    QSizeF size() const { return size_; }

    MindNode * node() const { return node_; }

public:
    // return new state
    bool toggle();

    void insertChild(MindNode const & node, MindNodeView * after = nullptr);

    void removeChild(MindNodeView * child);

    MindNodeView * findChild(MindNodeView * after = nullptr);

    void moveChild(MindNodeView * child, MindNodeView * toParent, MindNodeView * after = nullptr);

    void removeFromParent();

    void moveToParent(MindNodeView * toParent, MindNodeView * after = nullptr);

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
    MindViewStyle const * style_;
    QPointF pos_;
    QPointF pos2_; // sub tree topLeft
    QSizeF size_;

protected:
    MindNode * node_;
};

#endif // MINDNODEVIEW_H
