#include "inkstrokecontrol.h"
#include "inkstrokes.h"
#include "inkstrokehelper.h"

#include <views/qsshelper.h>
#include <views/whitecanvas.h>
#include <views/itemselector.h>
#include <core/resourcetransform.h>

#include <Windows/Controls/inkcanvas.h>
#include <Windows/Controls/inkevents.h>
#include <Windows/Ink/stroke.h>
#include <Windows/Input/StylusPlugIns/stylusplugincollection.h>
#include <Windows/Input/StylusPlugIns/stylusplugin.h>
#include <Windows/Input/StylusPlugIns/rawstylusinput.h>

#include <QElapsedTimer>
#include <QFrame>
#include <QGraphicsProxyWidget>
#include <QGraphicsScene>
#include <QGraphicsSceneEvent>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>

#ifndef QT_DEBUG
#define ERASE_CLIP_SHAPE 1
#else
#define ERASE_CLIP_SHAPE 1
#endif

InkStrokeControl::InkStrokeControl(ResourceView *res)
    : Control(res, {Touchable})
    , filterItem_(nullptr)
{
    if (res->flags().testFlag(ResourceView::Splittable)
            || qobject_cast<InkStrokes *>(res)->isClone()) {
        flags_.setFlag(FullLayout);
        flags_.setFlag(DefaultFlags, false);
    } else {
        flags_.setFlag(KeepAspectRatio, true);
    }
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
                setupMultiLayerErasing();
        } else {
            teardownMultiLayerErasing();
        }
        item_->removeSceneEventFilter(filterItem_);
        if (mode == InkCanvasEditingMode::Ink) {
            item_->installSceneEventFilter(filterItem_);
        }
        WhiteCanvas * canvas = whiteCanvas();
        canvas->removeSceneEventFilter(filterItem_);
        if (mode == InkCanvasEditingMode::None) {
            canvas->installSceneEventFilter(filterItem_);
            canvas->removeSceneEventFilter(canvas->selector());
            canvas->installSceneEventFilter(canvas->selector());
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
    void checkTip(QPointF const & pos);
    void showTip(QPointF const & pos);
private:
    QSharedPointer<int> life_;
    QElapsedTimer timer_;
    QGraphicsProxyWidget * tipItem_ = nullptr;
    int tipSeq_ = 0;
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
                    teardownMultiLayerErasing();
                    setupMultiLayerErasing();
                }
            }
        });
        Control * canvasControl = Control::fromItem(whiteCanvas());
        if (canvasControl) {
            connect(&canvasControl->resource()->transform(), &ResourceTransform::changed,
                    this, [ink] () {
                QTransform xf = ink->RenderTransform();
                xf = QTransform(xf.m11(), xf.m12(), xf.m21(), xf.m22(), 0, 0);
                ink->EraserShape()->SetTransform(xf.inverted().toAffine());
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
    auto l = life();
    ink->DefaultDrawingAttributes()->SetFitToCurve(false);
    ink->DefaultDrawingAttributes()->SetWidth(1.0);
    ink->DefaultDrawingAttributes()->SetHeight(1.0);
    strokes->load(ink->scene()->sceneRect().size(), ink->DefaultDrawingAttributes()).then([l, this, strokes, ink]() {
        if (l.isNull())
            return;
        ink->SetStrokes(strokes->strokes());
        ink->SetRenderSize(strokes->size());
        loadFinished(true);
    }, [this, l] (std::exception & e) {
        if (l.isNull())
            return;
        loadFinished(false, e.what());
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
        teardownMultiLayerErasing();
    }
}

bool EventFilterItem::sceneEventFilter(QGraphicsItem *watched, QEvent *event)
{
    if (watched != parentItem()) {
        if (event->type() == QEvent::GraphicsSceneMousePress
                && !event->isAccepted()) {
            QGraphicsSceneMouseEvent & me = static_cast<QGraphicsSceneMouseEvent&>(*event);
            checkTip(mapFromItem(watched, me.pos()));
        }
        return false;
    }
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
                me.accept();
                scene()->sendEvent(item, event);
                if (event->isAccepted()) {
                    setData(1000, QVariant::fromValue(item));
                    break;
                }
            }
        } else {
            QGraphicsItem * item = data(1000).value<QGraphicsItem *>();
            setData(1000, QVariant());
            if (items.contains(item)) {
                me.setPos(item->mapFromScene(me.scenePos()));
                me.setLastPos(item->mapFromScene(me.lastScenePos()));
                me.accept();
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

void EventFilterItem::checkTip(QPointF const & pos)
{
    if (timer_.isValid() && timer_.elapsed() < 1000) {
        showTip(pos);
        timer_.invalidate();
    } else {
        timer_.restart();
    }
}

void EventFilterItem::showTip(QPointF const & pos)
{
    if (tipItem_ == nullptr) {
        QWidget* widget = new QFrame;
        widget->setObjectName("inkstroketip");
        widget->setWindowFlag(Qt::FramelessWindowHint);
        widget->setStyleSheet(QssHelper(":/teachingtools/qss/inkstroketip.qss"));
        QLayout* layout = new QVBoxLayout(widget);
        widget->setLayout(layout);
        layout->setSpacing(20);
        layout->addWidget(new QLabel("是否切换到画笔?"));
        QPushButton * button = new QPushButton;
        button->setText("画笔");
        QObject::connect(button, &QPushButton::clicked, [this]() {
            qobject_cast<InkStrokeControl*>(Control::fromItem(parentItem()))
                    ->setEditingMode(InkCanvasEditingMode::Ink);
            tipItem_->hide();
        });
        layout->addWidget(button);
        QGraphicsProxyWidget * item = new QGraphicsProxyWidget(parentItem());
        item->setWidget(widget);
        item->setFlag(ItemIsFocusable, false);
        tipItem_ = item;
    }
    int tipSeq = ++tipSeq_;
    tipItem_->setPos(pos);
    tipItem_->show();
    QTimer::singleShot(3000, tipItem_->widget(), [this, tipSeq]() {
        if (tipSeq == tipSeq_)
            tipItem_->hide();
    });
}

void InkStrokeControl::setupMultiLayerErasing()
{
#if ERASE_CLIP_SHAPE
    QPolygonF clipShape;
#endif
    QList<QGraphicsItem*> items = item_->parentItem()->childItems();
    for (int i = items.size() - 1; i >= 0; --i) {
        Control * c = Control::fromItem(items[i]);
        if (InkStrokeControl* ic = qobject_cast<InkStrokeControl*>(c)) {
            if (ic != this) {
                InkCanvas * ink = static_cast<InkCanvas*>(items[i]);
#if ERASE_CLIP_SHAPE
                ink->SetEraseClip(clipShape);
#endif
            }
#if ERASE_CLIP_SHAPE
        } else {
            QPainterPath path = items[i]->type() == QGraphicsPathItem::Type
                    ? QPainterPath()
                    : items[i]->shape();
            QPolygonF shape = items[i]->mapToItem(item_, path.toFillPolygon());
            clipShape = clipShape.united(shape);
#endif
        }
    }
}

void InkStrokeControl::teardownMultiLayerErasing()
{
}
