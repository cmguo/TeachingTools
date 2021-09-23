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

    void moveFocus(int dir);

private:
    void update();

private:
    MindViewTemplate * template_;
    MindNodeView * rootView_;
    MindNodeView * focusedView_ = nullptr;
};

#endif // MINDMAPITEM_H
