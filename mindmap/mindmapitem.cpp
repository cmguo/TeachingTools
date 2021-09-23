#include "mindmapcontrol.h"
#include "mindmapitem.h"
#include "mindnode.h"
#include "mindnodeview.h"
#include "mindviewtemplate.h"
#include "simpleview.h"

#include <QEvent>
#include <QGraphicsSceneEvent>
#include <QKeyEvent>
#include <QDebug>
#include <QStyleOptionGraphicsItem>

MindMapItem::MindMapItem(QGraphicsItem *parent)
    : QGraphicsItem(parent)
{
    setAcceptHoverEvents(true);
    setAcceptedMouseButtons(Qt::LeftButton);
    setFlag(ItemIsFocusable);
    setFlag(ItemUsesExtendedStyleOption);

    template_ = new MindViewTemplate;
    rootView_ = new SimpleView(new MindNode);
    update();
}

void MindMapItem::load(MindViewTemplate *tpl, MindNode *node)
{
    delete template_;
    delete rootView_;
    template_ = tpl;
    rootView_ = tpl->createView(node);
    update();
}

bool MindMapItem::sceneEvent(QEvent *event)
{
    switch (event->type()) {
    case QEvent::GraphicsSceneMousePress:
        break;
    case QEvent::GraphicsSceneMouseMove:
        break;
    case QEvent::GraphicsSceneMouseRelease: {
        MindNodeView * view = rootView_->hitTest(static_cast<QGraphicsSceneMouseEvent*>(event)->pos());
        if (view)
            changeFocus(view);
        setFocus();
        break;
    }
    case QEvent::FocusIn:
        qDebug() << "MindMapItem FocusIn";
        break;
    case QEvent::FocusOut:
        qDebug() << "MindMapItem FocusOut";
        break;
    case QEvent::KeyRelease:
        switch (static_cast<QKeyEvent*>(event)->key()) {
        case Qt::Key_Enter:
            newNode(false);
            break;
        case Qt::Key_Tab:
            newNode(true);
            break;
        case Qt::Key_Backspace:
            removeFocusedNode();
            break;
        case Qt::Key_Left:
            moveFocus(MindNodeView::FocusLeft);
            break;
        case Qt::Key_Up:
            moveFocus(MindNodeView::FocusUp);
            break;
        case Qt::Key_Right:
            moveFocus(MindNodeView::FocusRight);
            break;
        case Qt::Key_Down:
            moveFocus(MindNodeView::FocusDown);
            break;
        default:
            return QGraphicsItem::sceneEvent(event);
        }
        break;
    default:
        return QGraphicsItem::sceneEvent(event);
    }
    return true;
}

QRectF MindMapItem::boundingRect() const
{
    return {0, 0, template_->xmax, template_->yoffset};
}

QPainterPath MindMapItem::shape() const
{
    QPainterPath s;
    rootView_->collectShape(s);
    return s;
}

bool MindMapItem::contains(const QPointF &point) const
{
    return rootView_->hitTest(point) != nullptr;
}

void MindMapItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *)
{
    painter->setPen(Qt::white);
    rootView_->draw(painter, option->exposedRect);
    if (focusedView_) {
        QRectF rc(focusedView_->pos(), focusedView_->size());
        painter->setPen(Qt::blue);
        painter->setBrush(QBrush());
        painter->drawRect(rc);
    }
}

void MindMapItem::toggle(MindNodeView *view)
{
    view->toggle();
    update();
}

void MindMapItem::newNode(bool childOrSiblin)
{
    if (focusedView_ == nullptr)
        return;
    if (childOrSiblin)
        focusedView_->newChild();
    else if (focusedView_->parent())
        focusedView_->parent()->newChild(focusedView_);
    else
        return;
    update();
}

void MindMapItem::removeFocusedNode()
{
    if (focusedView_ && focusedView_->parent()) {
        MindNodeView * next = focusedView_->nextFocus(MindNodeView::FocusUp);
        focusedView_->remove();
        focusedView_ = next;
        update();
    }
}

void MindMapItem::changeFocus(MindNodeView *view)
{
    if (focusedView_ == view)
        return;
    if (focusedView_) {
        QRectF rc(focusedView_->pos(), focusedView_->size());
        focusedView_ = view;
        QRectF rc2(focusedView_->pos(), focusedView_->size());
        QGraphicsItem::update(rc.united(rc2));
    } else {
        focusedView_ = view;
        QRectF rc2(focusedView_->pos(), focusedView_->size());
        QGraphicsItem::update(rc2);
    }
}

void MindMapItem::moveFocus(int dir)
{
    if (focusedView_ == nullptr)
        return;
    MindNodeView * next = focusedView_->nextFocus(static_cast<MindNodeView::FocusDirection>(dir));
    if (next == nullptr)
        return;
    changeFocus(next);
}

void MindMapItem::update()
{
    prepareGeometryChange();
    QPointF pos1 = rootView_->pos();
    rootView_->layout(*template_);
    QPointF pos2 = rootView_->pos();
    MindMapControl * control = qobject_cast<MindMapControl*>(Control::fromItem(this));
    if (control) {
        control->sizeChanged();
        QPointF diff = pos1 - pos2;
        control->move(diff);
    }
}
