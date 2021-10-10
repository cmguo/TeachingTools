#ifndef MINDNODEVIEW_H
#define MINDNODEVIEW_H

#include "mindbaseview.h"

#include <QList>
#include <QPointF>
#include <QSizeF>

class MindNode;
class MindViewTemplate;
class MindConnector;
class MindSwitch;
class MindSpacing;
class MindViewStyle;

class QPainterPath;

class MindNodeView : public MindBaseView
{
public:
    MindNodeView(MindNode * node);

    virtual ~MindNodeView();

public:
    virtual void layout(MindViewTemplate & tl);

    // call after layout
    virtual void collectShape(QPainterPath & shape);

    virtual bool isNode() const override { return true; }

    virtual QRectF boundingRect() const override;

    enum HitTestType {
        NodeOnly = 1,
        NodeSpacing = 3,
        SwitchOnly = 4,
        NodeSwitch = 5,
    };

    static MindSpacing HitTestSpacing;

    // call after layout
    virtual MindBaseView * hitTest(QPointF const & point, int types = NodeOnly);

    // call after layout
    virtual void draw(QPainter * painter, QRectF  const & exposedRect) override;

public:
    void setStyle(MindViewStyle const * style);

    MindViewStyle const * style() const { return style_; }

    QPointF pos() const { return pos_; }

    QRectF rect() const { return rect_; }

    QSizeF size() const { return size_; }

    QPointF inPort() const;

    QPointF outPort() const;

    QPointF switchPort() const;

    MindNode * node() const { return node_; }

    MindSwitch * _switch() { return this->switch_; }

public:
    void setTitle(QString const & title);

    // return new state
    bool toggle();

    void setEditing(bool b) { editing_ = b; }

public:
    int insertChild(MindNode const & node, MindNodeView * after = nullptr);

    void removeChild(MindNodeView * child);

    MindNodeView * childAt(int index);

    MindNodeView * findChildBefore(MindNodeView * before = nullptr);

    MindNodeView * findChildAfter(MindNodeView * after = nullptr);

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

private:
    int findChild(MindNodeView * node, bool beforeOrAfter);

    void moveBy(QPointF const & off);

protected:
    QList<QPair<MindNodeView*, MindConnector*>> children_;
    MindSwitch * switch_ = nullptr;
    MindViewStyle const * style_ = nullptr;
    QPointF pos_;
    QRectF rect_; // sub tree rect
    QSizeF size_;
    bool editing_ = false;

protected:
    MindNode * node_;
};

#endif // MINDNODEVIEW_H
