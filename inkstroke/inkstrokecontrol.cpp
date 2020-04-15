#include "inkstrokecontrol.h"
#include "inkstrokes.h"
#include "inkstrokehelper.h"

#include <views/whitecanvas.h>
#include <core/resourcetransform.h>

#include <Windows/Controls/inkcanvas.h>
#include <Windows/Controls/inkevents.h>
#include <Windows/Ink/stroke.h>
#include <Windows/Input/StylusPlugIns/stylusplugincollection.h>
#include <Windows/Input/StylusPlugIns/stylusplugin.h>
#include <Windows/Input/StylusPlugIns/rawstylusinput.h>

#include <QGraphicsScene>
#include <QGraphicsSceneEvent>

InkStrokeControl::InkStrokeControl(ResourceView *res)
    : Control(res, {FullLayout, Touchable}, DefaultFlags)
    , filterItem_(nullptr)
{
}

InkCanvasEditingMode InkStrokeControl::editingMode()
{
    InkCanvas * ink = static_cast<InkCanvas*>(item_);
    return ink->EditingMode();
}

QColor InkStrokeControl::color()
{
    InkCanvas * ink = static_cast<InkCanvas*>(item_);
    return ink->DefaultDrawingAttributes()->Color();
}

qreal InkStrokeControl::width()
{
    InkCanvas * ink = static_cast<InkCanvas*>(item_);
    return ink->DefaultDrawingAttributes()->Width();
}

void InkStrokeControl::setEditingMode(InkCanvasEditingMode mode)
{
    InkCanvas * ink = static_cast<InkCanvas*>(item_);
    if (mode == ink->EditingMode())
        return;
    ink->SetEditingMode(mode);
    item_->setAcceptHoverEvents(mode != InkCanvasEditingMode::None);
    if (res_->flags() & ResourceView::Splittable) {
#if !MIX_SELECT
        whiteCanvas()->enableSelector(mode == InkCanvasEditingMode::None);
#endif
        if (mode == InkCanvasEditingMode::EraseByPoint) {
            if (!whiteCanvas()->loading())
                setupErasing();
        } else {
            teardownErasing();
        }
        if (mode == InkCanvasEditingMode::Ink) {
            item_->installSceneEventFilter(filterItem_);
        } else {
            item_->removeSceneEventFilter(filterItem_);
        }
        editingModeChanged(mode);
    }
}

void InkStrokeControl::setColor(QColor c)
{
    InkCanvas * ink = static_cast<InkCanvas*>(item_);
    ink->DefaultDrawingAttributes()->SetColor(c);
}

void InkStrokeControl::setWidth(qreal w)
{
    InkCanvas * ink = static_cast<InkCanvas*>(item_);
    ink->DefaultDrawingAttributes()->SetWidth(w);
    ink->DefaultDrawingAttributes()->SetHeight(w);
}

void InkStrokeControl::clear()
{
    InkStrokes * strokes = qobject_cast<InkStrokes *>(res_);
    strokes->clear();
}

class EventFilterItem : public QGraphicsItem
{
public:
    EventFilterItem(QGraphicsItem * parentItem) : QGraphicsItem(parentItem), life_(new int) {}
    QRectF boundingRect() const override { return QRectF(); }
    void paint(QPainter *, const QStyleOptionGraphicsItem *, QWidget *) override {}
    bool sceneEventFilter(QGraphicsItem *watched, QEvent *event) override;
private:
    QSharedPointer<int> life_;
};

QGraphicsItem * InkStrokeControl::create(ResourceView *res)
{
    (void) res;
    InkCanvas * ink = InkStrokeHelper::createInkCanvas();
    if (!ink->acceptTouchEvents())
        flags_.setFlag(Touchable, false);
    ink->DefaultDrawingAttributes()->SetColor(Qt::white);
    filterItem_ = new EventFilterItem(ink);
    return ink;
}

void InkStrokeControl::attached()
{
    InkStrokes * strokes = qobject_cast<InkStrokes *>(res_);
    InkCanvas * ink = static_cast<InkCanvas*>(item_);
    if (res_->flags() & ResourceView::Splittable) {
        QObject::connect(strokes, &InkStrokes::cloned, this, [ink, strokes](){
            ink->SetStrokes(strokes->strokes()); // replace with empty strokes
        });
        QObject::connect(whiteCanvas(), &WhiteCanvas::loadingChanged, this, [this, ink](bool loading) {
            if (res_->flags() & ResourceView::Splittable) {
#if 0 // TODO: drive by data
                if (loading) {
                    setEditingMode(InkCanvasEditingMode::None);
                }
#endif
                if (!loading && ink->EditingMode() == InkCanvasEditingMode::EraseByPoint) {
                    teardownErasing();
                    setupErasing();
                }
            }
        });
        Control * canvasControl = Control::fromItem(whiteCanvas());
        if (canvasControl) {
            connect(&canvasControl->resource()->transform(), &ResourceTransform::changed,
                    this, [ink] () {
                ink->itemChange(QGraphicsItem::ItemTransformHasChanged, QVariant());
            });
        }
    }
    if (strokes->strokes()) {
        if (strokes->isClone())
            setEditingMode(InkCanvasEditingMode::None);
        ink->SetStrokes(strokes->strokes());
        loadFinished(true);
        return;
    }
    strokes->load(item_->boundingRect().size(), ink->DefaultDrawingAttributes()).then([life = life(), this, strokes, ink]() {
        if (life.isNull())
            return;
        if (strokes->strokes()) {
            ink->SetStrokes(strokes->strokes());
        }
        loadFinished(true);
    });
}

void InkStrokeControl::resize(const QSizeF &size)
{
    InkCanvas * ink = static_cast<InkCanvas*>(item_);
    ink->SetRenderSize(size);
}

Control::SelectMode InkStrokeControl::selectTest(QPointF const & pt)
{
    return InkStrokeHelper::selectTest(static_cast<InkCanvas*>(item_), pt, false, true);
}

void InkStrokeControl::detaching()
{
    if (res_->flags() & ResourceView::Splittable) {
        teardownErasing();
    }
}

bool EventFilterItem::sceneEventFilter(QGraphicsItem *watched, QEvent *event)
{
    (void) watched;
    if (event->type() == QEvent::GraphicsSceneMousePress
            || event->type() == QEvent::GraphicsSceneMouseRelease) {
        QGraphicsSceneMouseEvent & me = static_cast<QGraphicsSceneMouseEvent&>(*event);
        QList<QGraphicsItem*> items = scene()->items(me.scenePos());
        items = items.mid(items.indexOf(watched) + 1);
        QGraphicsItem* whiteCanvas = watched->parentItem()->parentItem();
        Qt::MouseEventSource source = me.source();
        me.setSource(Qt::MouseEventNotSynthesized);
        QPointF pos = me.pos();
        QPointF lastPos = me.lastPos();
        QWeakPointer<int> l = life_; // may switch page and destroyed
        if (event->type() == QEvent::GraphicsSceneMousePress) {
            for (QGraphicsItem * item : items) {
                if (item == whiteCanvas)
                    break;
                if (InkCanvas::fromItem(item))
                    break;
                me.setPos(item->mapFromScene(me.scenePos()));
                me.setLastPos(item->mapFromScene(me.lastScenePos()));
                if (scene()->sendEvent(item, event) && event->isAccepted()) {
                    setData(1000, QVariant::fromValue(item));
                    break;
                }
            }
        } else {
            QGraphicsItem * item = data(1000).value<QGraphicsItem *>();
            setData(1000, QVariant());
            if (items.contains(item)) {
                scene()->sendEvent(item, event);
            }
        }
        if (l.isNull())
            return true;
        me.setSource(source);
        me.setPos(pos);
        me.setLastPos(lastPos);
        event->ignore();
    }
    return false;
}

class InputBroadcaster : public StylusPlugIn
{
public:
    InputBroadcaster(InkCanvas* from, QList<InkCanvas*> to)
        : from_(from)
        , to_(to)
    {
        from_->StylusPlugIns().InsertItem(0, this);
    }

    virtual ~InputBroadcaster() override
    {
        from_->StylusPlugIns().RemoveItem(0);
    }

    virtual void OnStylusDown(RawStylusInput &rawStylusInput) override
    {
        broadcast(rawStylusInput);
    }

    virtual void OnStylusMove(RawStylusInput &rawStylusInput) override
    {
        broadcast(rawStylusInput);
    }

    virtual void OnStylusUp(RawStylusInput &rawStylusInput) override
    {
        broadcast(rawStylusInput);
    }

private:
    void broadcast(RawStylusInput &rawStylusInput)
    {
        QEvent& e(rawStylusInput.inputEvent());
        for (InkCanvas* ink : to_) {
            ink->sceneEvent(&e);
        }
    }

private:
    InkCanvas* from_;
    QList<InkCanvas*> to_;
};

Q_DECLARE_METATYPE(InputBroadcaster*)

void InkStrokeControl::setupErasing()
{
    QPolygonF clipShape;
    QList<InkCanvas*> list;
    QList<QGraphicsItem*> items = item_->parentItem()->childItems();
    for (int i = items.size() - 1; i >= 0; --i) {
        Control * c = Control::fromItem(items[i]);
        if (InkStrokeControl* ic = qobject_cast<InkStrokeControl*>(c)) {
            if (ic != this) {
                InkCanvas * ink = static_cast<InkCanvas*>(items[i]);
                ink->SetEditingMode(InkCanvasEditingMode::EraseByPoint);
                ink->SetEraseClip(clipShape);
                list.append(ink);
            }
        } else {
            QPainterPath path = items[i]->type() == QGraphicsPathItem::Type
                    ? QPainterPath()
                    : items[i]->shape();
            QPolygonF shape = items[i]->mapToItem(item_, path.toFillPolygon());
            clipShape = clipShape.united(shape);
        }
    }
    InkCanvas * ink = static_cast<InkCanvas*>(item_);
    InputBroadcaster* ib = new InputBroadcaster(ink, list);
    setProperty("InputBroadcaster", QVariant::fromValue(ib));
}

void InkStrokeControl::teardownErasing()
{
    QVariant ib = property("InputBroadcaster");
    if (!ib.isValid())
        return;
    delete ib.value<InputBroadcaster*>();
    ib.clear();
    setProperty("InputBroadcaster", ib);
    for (QGraphicsItem* i : item_->parentItem()->childItems()) {
        Control * c = Control::fromItem(i);
        InkStrokeControl* ic = qobject_cast<InkStrokeControl*>(c);
        if (ic) {
            if (ic != this) {
                ic->setEditingMode(InkCanvasEditingMode::None);
            }
        }
    }
}
