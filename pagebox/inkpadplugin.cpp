#include "inkpadplugin.h"
#include "pageboxdocitem.h"
#include "inkstroke/inkstrokehelper.h"

#include <core/toolbutton.h>

#include <Windows/Controls/inkcanvas.h>
#include <Windows/Controls/inkevents.h>
#include <Windows/Ink/drawingattributes.h>

#include <QGraphicsProxyWidget>
#include <QPen>

INKCANVAS_USE_NAMESPACE

InkPadPlugin::InkPadPlugin(QObject * parent)
    : PageBoxPlugin(parent)
{
    inkCanvas_ = InkStrokeHelper::createInkCanvas(QColor("#FFFF6262"), 4.0);
    inkCanvas_->SetEditingMode(InkCanvasEditingMode::Ink);
    item_ = inkCanvas_;
    setToolsString(InkStrokeHelper::toolString());
}

InkPadPlugin::InkPadPlugin(const InkPadPlugin &o)
{
    inkCanvas_ = InkStrokeHelper::createInkCanvas(QColor("#FFFF6262"), 4.0);
    item_ = inkCanvas_;
    pageStrokes_ = o.pageStrokes_;
    for (QSharedPointer<StrokeCollection> & s : pageStrokes_)
        s = s->Clone();
}

void InkPadPlugin::stroke()
{
    //if (inkCanvas_->EditingMode() == InkCanvasEditingMode::Ink)
    //    inkCanvas_->SetEditingMode(InkCanvasEditingMode::None);
    //else
        inkCanvas_->SetEditingMode(InkCanvasEditingMode::Ink);
}

void InkPadPlugin::stroke(QString const & arg)
{
    QVariant v(arg);
    if (arg.startsWith("#")) {
        v.convert(QVariant::Color);
        inkCanvas_->DefaultDrawingAttributes()->SetColor(v.value<QColor>());
    } else {
        v.convert(QVariant::Double);
        inkCanvas_->DefaultDrawingAttributes()->SetWidth(v.toDouble());
        inkCanvas_->DefaultDrawingAttributes()->SetHeight(v.toDouble());
    }
}

void InkPadPlugin::eraser()
{
    inkCanvas_->SetEditingMode(InkCanvasEditingMode::EraseByPoint);
    inkCanvas_->itemChange(QGraphicsItem::ItemTransformHasChanged, QVariant()); // for update cursor
}

void InkPadPlugin::eraser(const QString &)
{
    inkCanvas_->Strokes()->ClearItems();
}

void InkPadPlugin::onRelayout(int pageCount)
{
    pageStrokes_.clear();
    pageStrokes_.resize(pageCount);
}

void InkPadPlugin::onPageChanged(int lastPage, int curPage)
{
    (void) lastPage;
    if (document()->layoutMode() == PageBoxDocItem::Continuous)
        curPage = 0;
    QSharedPointer<StrokeCollection> & strokes = pageStrokes_[curPage];
    if (!strokes)
        strokes.reset(new StrokeCollection);
    inkCanvas_->SetStrokes(strokes);
}

void InkPadPlugin::onSizeChanged(const QSizeF &, const QSizeF &pageSize)
{
    QSizeF size = document()->mapToScene(QRectF({0, 0}, pageSize)).boundingRect().size();
    inkCanvas_->setScale(pageSize.width() / size.width());
    inkCanvas_->SetRenderSize(size);
}

bool InkPadPlugin::selectTest(const QPointF &pt)
{
    return InkStrokeHelper::selectTest(inkCanvas_, pt) != Control::NotSelect;
}

void InkPadPlugin::getToolButtons(QList<ToolButton *> &buttons, ToolButton *parent)
{
    if (parent == nullptr) {
        PageBoxPlugin::getToolButtons(buttons, parent);
        return;
    }
    InkStrokeHelper::getToolButtons(inkCanvas_, buttons, parent);
}

void InkPadPlugin::updateToolButton(ToolButton *button)
{
    InkStrokeHelper::updateToolButton(inkCanvas_, button);
}
