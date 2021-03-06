#include "inkstrokecontrol.h"
#include "inkstrokes.h"
#include "inkstrokehelper.h"
#include "inkstrokefilter.h"

#include <views/whitecanvas.h>
#include <views/itemselector.h>
#include <widget/toolbarwidget.h>
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

INKCANVAS_USE_NAMESPACE

#ifdef QT_DEBUG
#define ERASE_CLIP_SHAPE 1
#else
#define ERASE_CLIP_SHAPE 0
#endif

static constexpr char const * toolstr =
        "fitToCurve()|FTC|Checkable,NeedUpdate|;"
        ;


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
        setToolsString(toolstr);
    }
}

InkCanvasEditingMode InkStrokeControl::editingMode()
{
    InkCanvas * ink = static_cast<InkCanvas*>(item_);
    return ink->EditingMode();
}

InkStrokeGeometry::Shape InkStrokeControl::shapeMode() const
{
    InkCanvas * ink = static_cast<InkCanvas*>(item_);
    return ink->findChild<InkStrokeGeometry*>()->shapeMode();
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
        // focus defeats click pass throught
        item_->setFlag(QGraphicsItem::ItemIsFocusable,
                       mode != InkCanvasEditingMode::Ink);
        item_->removeSceneEventFilter(filterItem_);
        if (mode == InkCanvasEditingMode::Ink) {
            item_->installSceneEventFilter(filterItem_);
        }
        WhiteCanvas * canvas = whiteCanvas();
        canvas->removeSceneEventFilter(filterItem_);
        if (mode == InkCanvasEditingMode::None) {
            canvas->installSceneEventFilter(filterItem_);
            // selector first
            QTimer::singleShot(0, this, [canvas]() {
                canvas->removeSceneEventFilter(canvas->selector());
                canvas->installSceneEventFilter(canvas->selector());
            });
        }
        editingModeChanged(mode);
    }
}

void InkStrokeControl::setShapeMode(InkStrokeGeometry::Shape shape)
{
    InkCanvas * ink = static_cast<InkCanvas*>(item_);
    ink->findChild<InkStrokeGeometry*>()->setShapeMode(shape);
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

void InkStrokeControl::stylusTip()
{
    InkCanvas * ink = static_cast<InkCanvas*>(item_);
    QSharedPointer<StrokeCollection> selection = ink->GetSelectedStrokes();
    for (QSharedPointer<Stroke> s : *selection) {
        if (s->GetDrawingAttributes()->GetStylusTip() == StylusTip::Ellipse)
            s->GetDrawingAttributes()->SetStylusTip(StylusTip::Rectangle);
        else
            s->GetDrawingAttributes()->SetStylusTip(StylusTip::Ellipse);
    }
}

void InkStrokeControl::fitToCurve()
{
    InkCanvas * ink = static_cast<InkCanvas*>(item_);
    QSharedPointer<StrokeCollection> selection = ink->GetSelectedStrokes();
    for (QSharedPointer<Stroke> s : *selection) {
        s->GetDrawingAttributes()->SetFitToCurve(
                    !s->GetDrawingAttributes()->FitToCurve());
    }
}

ControlView *InkStrokeControl::create(ControlView *parent)
{
    (void) parent;
    InkCanvas * ink = InkStrokeHelper::createInkCanvas();
    if (!ink->acceptTouchEvents())
        flags_.setFlag(Touchable, false);
    ink->DefaultDrawingAttributes()->SetColor(Qt::white);
    if (res_->flags().testFlag(ResourceView::Splittable)) {
        filterItem_ = new InkStrokeFilter(ink);
        InkStrokeHelper::enableStylusGusture(ink);
    }
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
                QMatrix xf = ink->RenderTransform();
                xf = QMatrix(xf.m11(), xf.m12(), xf.m21(), xf.m22(), 0, 0);
                ink->EraserShape()->SetTransform(xf.inverted());
                ink->itemChange(QGraphicsItem::ItemTransformHasChanged, QVariant());
            });
        }
        WhiteCanvas * canvas = whiteCanvas();
        if (editingMode() == InkCanvasEditingMode::None) {
            canvas->installSceneEventFilter(filterItem_);
            // selector first
            canvas->removeSceneEventFilter(canvas->selector());
            canvas->installSceneEventFilter(canvas->selector());
        }
    } // Splittable
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
    }, [this, l] (std::exception &) {
        if (l.isNull())
            return;
        loadFailed();
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
