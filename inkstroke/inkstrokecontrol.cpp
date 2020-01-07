#include "inkstrokecontrol.h"
#include "inkstrokes.h"
#include "inkstrokehelper.h"

#include <views/whitecanvas.h>

#include <Windows/Controls/inkcanvas.h>
#include <Windows/Controls/inkevents.h>
#include <Windows/Ink/stroke.h>

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
    whiteCanvas()->enableSelector(mode == InkCanvasEditingMode::None);
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
    InkCanvas * ink = InkStrokeHelper::createInkCanvas();
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
    return InkStrokeHelper::selectTest(static_cast<InkCanvas*>(item_), pt, false);
}
