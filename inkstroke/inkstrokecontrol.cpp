#include "inkstrokecontrol.h"
#include "inkstrokes.h"
#include <Windows/Controls/inkcanvas.h>
#include <Windows/Controls/inkevents.h>
#include <Windows/Ink/stroke.h>

#include <QGraphicsItem>

InkStrokeControl::InkStrokeControl(ResourceView *res)
    : Control(res, FullLayout, DefaultFlags)
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
    ink->SetEditingMode(mode);
    item_->setAcceptHoverEvents(mode != InkCanvasEditingMode::None);
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
    InkCanvas * ink = static_cast<InkCanvas*>(item_);
    ink->Strokes()->ClearItems();
}

QGraphicsItem * InkStrokeControl::create(ResourceView *res)
{
    (void) res;
    InkCanvas * ink = createInkCanvas();
    ink->DefaultDrawingAttributes()->SetColor(Qt::white);
    return ink;
}

void InkStrokeControl::attached()
{
    InkStrokes * strokes = qobject_cast<InkStrokes *>(res_);
    InkCanvas * ink = static_cast<InkCanvas*>(item_);
    if (res_->flags() & ResourceView::Splittable) {
        QObject::connect(strokes, &InkStrokes::cloned, this, [ink, strokes](){
            ink->SetStrokes(strokes->strokes());
        });
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
    return selectTest(static_cast<InkCanvas*>(item_), pt, false);
}


class PressureHelper : public QObject
{
public:
    PressureHelper(InkCanvas* ink)
        : QObject(ink)
        , ink_(ink)
    {
        ink_->AddHandler(InkCanvas::StrokeCollectedEvent, RoutedEventHandlerT<
                        PressureHelper, InkCanvasStrokeCollectedEventArgs, &PressureHelper::applyPressure>(this));
    }

    ~PressureHelper()
    {
        ink_->RemoveHandler(InkCanvas::StrokeCollectedEvent, RoutedEventHandlerT<
                        PressureHelper, InkCanvasStrokeCollectedEventArgs, &PressureHelper::applyPressure>(this));
    }

private:
    void applyPressure(InkCanvasStrokeCollectedEventArgs &e)
    {
        QSharedPointer<Stroke> stroke = e.GetStroke();
        QSharedPointer<StylusPointCollection> stylusPoints = stroke->StylusPoints()->Clone();
        int n = 16;
        if (stylusPoints->size() > n) {
            for (int i = 1; i < n; ++i) {
                int m = stylusPoints->size() + i - n;
                StylusPoint point = (*stylusPoints)[m];
                float d = static_cast<float>(i) / static_cast<float>(n);
                point.SetPressureFactor(point.PressureFactor() * (1.0f - d * d));
                stylusPoints->SetItem(m, point);
            }
            --n;
        } else {
            n = 0;
        }
        //QUuid guid("52053C24-CBDD-4547-AAA1-DEFEBF7FD1E1");
        //stroke->AddPropertyData(guid, 2.0);
        stroke->SetStylusPoints(stylusPoints);
    }

private:
    InkCanvas * ink_;
};

InkCanvas *InkStrokeControl::createInkCanvas(qreal lineWidth)
{
    InkCanvas * ink = new InkCanvas;
    //ink->setStyleSheet("background-color:red;");
    //ink->DefaultDrawingAttributes()->SetStylusTip(StylusTip::Rectangle);
    ink->DefaultDrawingAttributes()->SetFitToCurve(true);
    ink->DefaultDrawingAttributes()->SetWidth(lineWidth);
    ink->DefaultDrawingAttributes()->SetHeight(lineWidth);
    ink->SetEditingMode(InkCanvasEditingMode::None);
    new PressureHelper(ink); // attached to InkCanvas
    return ink;
}

Control::SelectMode InkStrokeControl::selectTest(InkCanvas *ink, const QPointF &pt, bool eatUnselect)
{
    if (ink->EditingMode() == InkCanvasEditingMode::None) {
        InkCanvasSelectionHitResult result = ink->HitTestSelection(pt);
        if (result != InkCanvasSelectionHitResult::None)
            return NotSelect;
        QSharedPointer<StrokeCollection> hits = ink->Strokes()->HitTest(pt);
        if (hits && !hits->empty()) {
            ink->Select(hits);
            ink->setProperty("tempSelect", true);
            return NotSelect;
        }
        return PassSelect;
    } else if (ink->EditingMode() == InkCanvasEditingMode::Select &&
               ink->property("tempSelect").isValid()) {
        InkCanvasSelectionHitResult result = ink->HitTestSelection(pt);
        if (result == InkCanvasSelectionHitResult::None) {
            QSharedPointer<StrokeCollection> hits = ink->Strokes()->HitTest(pt);
            if (hits && !hits->empty()) {
                ink->Select(hits);
                return NotSelect;
            }
            ink->setProperty("tempSelect", QVariant());
            ink->SetEditingMode(InkCanvasEditingMode::None);
            return eatUnselect ? NotSelect : PassSelect;
        } else {
            return NotSelect;
        }
    } else {
        return NotSelect;
    }
}

