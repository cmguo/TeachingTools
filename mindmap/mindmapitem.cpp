#include "mindmapcontrol.h"
#include "mindmapitem.h"
#include "mindnode.h"
#include "mindnodeview.h"
#include "mindviewtemplate.h"
#include "mindviewstyle.h"
#include "simpleview.h"

#include <QEvent>
#include <QGraphicsSceneEvent>
#include <QKeyEvent>
#include <QDebug>
#include <QStyleOptionGraphicsItem>
#include <QDrag>
#include <QMimeData>

static QRectF viewRect(MindNodeView * v1, MindNodeView * v2);

static MindViewTemplate tpl_empty;
static MindNode node_empty;
static MindNodeView dropMiddle(&node_empty);

MindMapItem::MindMapItem(QGraphicsItem *parent)
    : QGraphicsItem(parent)
{
    setAcceptHoverEvents(true);
    setAcceptedMouseButtons(Qt::LeftButton);
    setAcceptDrops(true);
    setFlag(ItemIsFocusable);
    setFlag(ItemUsesExtendedStyleOption);

    load(&tpl_empty, &node_empty);
}

void MindMapItem::load(MindViewTemplate *tpl, MindNode *node)
{
    if (rootView_)
        delete rootView_;
    template_ = tpl;
    rootView_ = tpl->createView(node);
    update();
}

bool MindMapItem::sceneEvent(QEvent *event)
{
    switch (event->type()) {
    case QEvent::GraphicsSceneMousePress:
        moveView_ = rootView_->hitTest(static_cast<QGraphicsSceneMouseEvent*>(event)->pos());
        moveStart_ = static_cast<QGraphicsSceneMouseEvent*>(event)->scenePos();
        moved_ = false;
        break;
    case QEvent::GraphicsSceneMouseMove: {
        QPointF d = static_cast<QGraphicsSceneMouseEvent*>(event)->scenePos() - moveStart_;
        if (moved_ || qAbs(d.x()) + qAbs(d.y()) > 10) {
            if (moveView_ == rootView_ || moveView_ == nullptr) {
                MindMapControl * control = qobject_cast<MindMapControl*>(Control::fromItem(this));
                control->move(d);
            } else if (!moved_) {
                moveStart_ = mapFromScene(moveStart_);
                dragStart(moveView_);
            }
            moved_ = true;
            moveStart_ += d;
        }
        break;
    }
    case QEvent::GraphicsSceneMouseRelease: {
        if (moved_) {

        } else {
            if (moveView_)
                changeFocus(moveView_);
        }
        moveView_ = nullptr;
        moved_ = false;
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
        case Qt::Key_Return:
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
    case QEvent::GraphicsSceneDragEnter:
        static_cast<QGraphicsSceneDragDropEvent*>(event)->setAccepted(
                    static_cast<QGraphicsSceneDragDropEvent*>(event)->mimeData()->hasFormat("mindnode"));
        static_cast<QGraphicsSceneDragDropEvent*>(event)->setDropAction(Qt::DropAction::MoveAction);
        break;
    case QEvent::GraphicsSceneDragMove: {
        dropMiddle.setParent(moveView_);
        MindNodeView * targetView = rootView_->hitTest(
                    static_cast<QGraphicsSceneDragDropEvent*>(event)->pos() - moveStart_ + moveView_->pos(), &dropMiddle);
        if (targetView == moveView_) targetView= nullptr;
        QGraphicsItem::update(viewRect(targetView_, targetView));
        targetView_ = targetView;
        static_cast<QGraphicsSceneDragDropEvent*>(event)->accept();
        break;
    }
    case QEvent::GraphicsSceneDrop: {
        moveNode();
        targetView_ = nullptr;
        moveView_ = nullptr;
        static_cast<QGraphicsSceneDragDropEvent*>(event)->accept();
        break;
    }
    default:
        return QGraphicsItem::sceneEvent(event);
    }
    return true;
}

QRectF MindMapItem::boundingRect() const
{
    return {0, 0, template_->xmax, template_->yoffset};
}

bool MindMapItem::contains(const QPointF &point) const
{
    return rootView_->hitTest(point) != nullptr;
}

void MindMapItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *)
{
    painter->setPen(Qt::NoPen);
    painter->setBrush(Qt::white);
    painter->drawRect(option->exposedRect);
    rootView_->draw(painter, option->exposedRect);
    if (focusedView_) {
        QRectF rc(focusedView_->pos(), focusedView_->size());
        rc.adjust(2, 2, -2, -2);
        painter->setPen(QPen(Qt::blue, 4));
        painter->setBrush(QBrush());
        painter->drawRect(rc);
    }
    if (targetView_) {
        QRectF rc(targetView_->pos(), targetView_->size());
        rc.adjust(2, 2, -2, -2);
        painter->setPen(QPen(Qt::yellow, 4));
        painter->setBrush(QBrush());
        painter->drawRect(rc);
    }
}

void MindMapItem::toggle(MindNodeView *view)
{
    if (!view->toggle()) {
        if (focusedView_ && focusedView_->hasParent(view))
            focusedView_ = nullptr;
    }
    update();
}

void MindMapItem::newNode(bool childOrSiblin)
{
    if (focusedView_ == nullptr)
        return;
    MindNodeView * parent = focusedView_->parent();
    MindNodeView * after = nullptr;
    if (childOrSiblin)
        parent = focusedView_;
    else if (parent)
        after = focusedView_;
    else
        return;
    parent->insertChild(template_->createNode(), after);
    update();
    focusedView_ = parent->findChild(after);
}

void MindMapItem::removeFocusedNode()
{
    if (focusedView_ && focusedView_->parent()) {
        MindNodeView * next = focusedView_->nextFocus(MindNodeView::FocusUp);
        focusedView_->removeFromParent();
        focusedView_ = next;
        update();
    }
}

void MindMapItem::moveNode()
{
    if (targetView_ == &dropMiddle)
        moveView_->moveToParent(targetView_->parent()->parent(), targetView_->parent());
    else if (targetView_)
        moveView_->moveToParent(targetView_);
    update();
}

void MindMapItem::changeFocus(MindNodeView *view)
{
    if (focusedView_ == view)
        return;
    QGraphicsItem::update(viewRect(focusedView_, view));
    focusedView_ = view;
}

void MindMapItem::dragStart(MindNodeView *view)
{
    QDrag * drag = new QDrag(Control::fromItem(this));
    QMimeData *mimeData = new QMimeData;
    mimeData->setData("mindnode", QByteArray::number(reinterpret_cast<uintptr_t>(view)));
    drag->setMimeData(mimeData);
    QPixmap pixmap(view->size().toSize());
    pixmap.fill(Qt::transparent);
    QPainter painter(&pixmap);
    QRectF rc {{0, 0}, view->size()};
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setPen(QPen(Qt::darkGray, 2));
    painter.setBrush(Qt::gray);
    painter.drawRoundedRect(rc, 5, 5);
    painter.setFont(view->style()->textFont());
    painter.setPen(view->style()->textColor());
    painter.drawText(rc, Qt::AlignCenter, view->node()->title);
    painter.end();
    drag->setPixmap(pixmap);
    drag->setHotSpot((moveStart_ - view->pos()).toPoint());
    drag->exec();
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

bool MindMapItem::hitTestRoot(const QPointF &point)
{
    return rootView_->hitTest(point) == rootView_;
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

static QRectF viewRect(MindNodeView * v1, MindNodeView * v2)
{
    if (v1 == nullptr) {
        if (v2 == nullptr)
            return {};
        return {v2->pos(), v2->size()};
    } else {
        if (v2 == nullptr)
            return {v1->pos(), v1->size()};
        else
            return QRectF{v1->pos(), v1->size()} | QRectF{v2->pos(), v2->size()};
    }
}
