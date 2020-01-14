#include "inkstrokecontrol.h"
#include "inkstrokes.h"
#include "inkstrokehelper.h"

#include <views/whitecanvas.h>

#include <Windows/Controls/inkcanvas.h>
#include <Windows/Controls/inkevents.h>
#include <Windows/Ink/stroke.h>
#include <Windows/Input/StylusPlugIns/stylusplugincollection.h>
#include <Windows/Input/StylusPlugIns/stylusplugin.h>
#include <Windows/Input/StylusPlugIns/rawstylusinput.h>

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
    if (res_->flags() & ResourceView::Splittable) {
        if (mode == InkCanvasEditingMode::EraseByPoint) {
            if (!whiteCanvas()->loading())
                setupErasing();
        } else {
            teardownErasing();
        }
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
    ink->DefaultDrawingAttributes()->SetColor(Qt::white);
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
        QObject::connect(whiteCanvas(), &WhiteCanvas::loadFinished, this, [this, ink]() {
            if (res_->flags() & ResourceView::Splittable) {
                if (ink->EditingMode() == InkCanvasEditingMode::EraseByPoint) {
                    teardownErasing();
                    setupErasing();
                }
            }
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

void InkStrokeControl::detaching()
{
    if (res_->flags() & ResourceView::Splittable) {
        teardownErasing();
    }
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
        InkStrokeControl* ic = qobject_cast<InkStrokeControl*>(c);
        if (ic) {
            if (ic != this) {
                InkCanvas * ink = static_cast<InkCanvas*>(items[i]);
                ink->SetEditingMode(InkCanvasEditingMode::EraseByPoint);
                ink->SetEraseClip(clipShape);
                list.append(ink);
            }
        } else {
            QPolygonF shape = items[i]->mapToItem(item_, items[i]->shape().toFillPolygon());
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
