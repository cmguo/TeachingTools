#ifndef MINDMAPITEM_H
#define MINDMAPITEM_H

#include <QGraphicsItem>

class MindNode;
class MindNodeView;
class MindViewTemplate;

class MindMapItem : public QGraphicsItem
{
public:
    MindMapItem(QGraphicsItem * parent = nullptr);

    void load(MindViewTemplate * tpl, MindNode * node);

protected:
    virtual bool sceneEvent(QEvent *event) override;

    virtual QRectF boundingRect() const override;

    virtual QPainterPath shape() const override;

    virtual bool contains(const QPointF &point) const override;

    virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = nullptr) override;

public:
    void toggle(MindNodeView * view);

    void newNode(bool childOrSiblin);

    void removeFocusedNode();

    void changeFocus(MindNodeView * view);

    void dragStart(MindNodeView * view);

    void moveFocus(int dir);

    bool hitTestRoot(const QPointF &point);

private:
    void update();

private:
    MindViewTemplate * template_ = nullptr;
    MindNodeView * rootView_ = nullptr;
    MindNodeView * focusedView_ = nullptr;
    QPainterPath shape_;

    MindNodeView * moveView_ = nullptr;
    MindNodeView * targetView_ = nullptr;
    QPointF moveStart_;
    bool moved_ = false;
};

#endif // MINDMAPITEM_H
