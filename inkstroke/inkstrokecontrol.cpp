#include "inkstrokecontrol.h"
#include "inkstrokes.h"
#include <Windows/Controls/inkcanvas.h>
#include <Windows/Controls/inkevents.h>
#include <Windows/Ink/stroke.h>

REGISTER_CONTROL(InkStrokeControl, "inkstroke");
REGISTER_RESOURCE_VIEW(InkStrokes, "inkstroke");

InkStrokeControl::InkStrokeControl(ResourceView *res)
    : WidgetControl(res, FullLayout, DefaultFlags)
    , tempSelect_(false)
{
}

InkCanvasEditingMode InkStrokeControl::editingMode()
{
    InkCanvas * ink = static_cast<InkCanvas*>(widget_);
    return ink->EditingMode();
}

void InkStrokeControl::setEditingMode(InkCanvasEditingMode mode)
{
    InkCanvas * ink = static_cast<InkCanvas*>(widget_);
    ink->SetEditingMode(mode);
}

QWidget *InkStrokeControl::createWidget(ResourceView *res)
{
    (void) res;
    return createInkCanvas();
}

void InkStrokeControl::attached()
{
    InkStrokes * strokes = qobject_cast<InkStrokes *>(res_);
    InkCanvas * ink = static_cast<InkCanvas*>(widget_);
    if (!strokes->isClone()) {
        ink->AddHandler(InkCanvas::StrokeCollectedEvent, RoutedEventHandlerT<
                        InkStrokeControl, InkCanvasStrokeCollectedEventArgs, &InkStrokeControl::onStrokeCollected>(this));
    }
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
    strokes->load(widget_->size(), ink->DefaultDrawingAttributes()).then([life = life(), this, strokes, ink]() {
        if (life.isNull())
            return;
        if (strokes->strokes()) {
            ink->SetStrokes(strokes->strokes());
        }
        loadFinished(true);
    });
}

void InkStrokeControl::detaching()
{
    InkCanvas * ink = static_cast<InkCanvas*>(widget_);
    ink->RemoveHandler(InkCanvas::StrokeCollectedEvent, RoutedEventHandlerT<
                    InkStrokeControl, InkCanvasStrokeCollectedEventArgs, &InkStrokeControl::onStrokeCollected>(this));
}

Control::SelectMode InkStrokeControl::selectTest(QPointF const & pt)
{
    InkCanvas * ink = static_cast<InkCanvas*>(widget_);
    if (editingMode() == InkCanvasEditingMode::None) {
        InkCanvasSelectionHitResult result = ink->HitTestSelection(pt);
        if (result != InkCanvasSelectionHitResult::None)
            return NotSelect;
        QSharedPointer<StrokeCollection> hits = ink->Strokes()->HitTest(pt);
        if (hits && !hits->empty()) {
            ink->Select(hits);
            tempSelect_ = true;
            return NotSelect;
        }
        return PassSelect;
    } else if (editingMode() == InkCanvasEditingMode::Select && tempSelect_) {
        InkCanvasSelectionHitResult result = ink->HitTestSelection(pt);
        if (result == InkCanvasSelectionHitResult::None) {
            ink->SetEditingMode(InkCanvasEditingMode::None);
            return PassSelect;
        } else {
            return NotSelect;
        }
    } else {
        return NotSelect;
    }
}

void InkStrokeControl::onStrokeCollected(InkCanvasStrokeCollectedEventArgs &e)
{
    applyPressure(e);
}

InkCanvas *InkStrokeControl::createInkCanvas(qreal lineWidth)
{
    InkCanvas * ink = new InkCanvas;
    //ink->setStyleSheet("background-color:red;");
    //ink->DefaultDrawingAttributes()->SetStylusTip(StylusTip::Rectangle);
    ink->DefaultDrawingAttributes()->SetFitToCurve(true);
    ink->DefaultDrawingAttributes()->SetWidth(lineWidth);
    ink->DefaultDrawingAttributes()->SetHeight(lineWidth);
    ink->SetEditingMode(InkCanvasEditingMode::None);
    ink->setAttribute(Qt::WA_NoSystemBackground);
    return ink;
}

void InkStrokeControl::applyPressure(InkCanvasStrokeCollectedEventArgs &e)
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

