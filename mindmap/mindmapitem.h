#ifndef MINDMAPITEM_H
#define MINDMAPITEM_H

#include <QGraphicsItem>

class MindNode;
class MindNodeView;
class MindBaseView;
class MindViewTemplate;
class MindSwitch;

class MindTextEditItem;

class MindMapItem : public QGraphicsItem
{
public:
    MindMapItem(QGraphicsItem * parent = nullptr);

    void load(MindViewTemplate * tpl, MindNode * node);

protected:
    virtual bool sceneEvent(QEvent *event) override;

    virtual QRectF boundingRect() const override;

    virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = nullptr) override;

public:
    void toggle(MindNodeView * view);

    void newNode(bool childOrSiblin);

    void removeFocusedNode();

    void moveNode();

    void changeFocus(MindNodeView * view);

    void dragStart(MindNodeView * view);

    void moveFocus(int dir);

    bool hitTestRoot(const QPointF &point);

    void hoveSwitch(MindSwitch * sw);

private:
    void updateLayout();

private:
    MindViewTemplate * template_ = nullptr;
    MindNodeView * rootView_ = nullptr;
    MindNodeView * focusedView_ = nullptr;
    MindTextEditItem * editItem_;

    MindBaseView * moveView_ = nullptr;
    MindBaseView * targetView_ = nullptr;
    MindSwitch * hoveredSwitch_ = nullptr;
    QPointF moveStart_;
    bool moved_ = false;
};

#endif // MINDMAPITEM_H
