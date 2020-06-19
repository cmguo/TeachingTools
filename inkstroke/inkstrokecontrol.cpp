#include "inkstrokecontrol.h"
#include "inkstrokes.h"
#include "inkstrokehelper.h"
#include "inkstrokefilter.h"

#include <views/whitecanvas.h>
#include <views/itemselector.h>
#include <views/toolbarwidget.h>
#include <core/resourcetransform.h>

#include <Windows/Controls/inkcanvas.h>
#include <Windows/Controls/inkevents.h>
#include <Windows/Ink/stroke.h>
#include <Windows/Input/StylusPlugIns/stylusplugincollection.h>
#include <Windows/Input/StylusPlugIns/stylusplugin.h>
#include <Windows/Input/StylusPlugIns/rawstylusinput.h>

#include <QApplication>
#include <QGraphicsView>
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
            // selector first
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

QGraphicsItem * InkStrokeControl::create(ResourceView *res)
{
    (void) res;
    InkCanvas * ink = InkStrokeHelper::createInkCanvas();
    if (!ink->acceptTouchEvents())
        flags_.setFlag(Touchable, false);
    ink->DefaultDrawingAttributes()->SetColor(Qt::white);
    filterItem_ = new InkStrokeFilter(ink);
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
                ink->SetEditMask(clipShape);
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
