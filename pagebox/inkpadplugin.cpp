#include "inkpadplugin.h"
#include "pageboxdocitem.h"
#include "inkstroke/inkstrokecontrol.h"

#include <Windows/Controls/inkcanvas.h>
#include <Windows/Controls/inkevents.h>

#include <QGraphicsProxyWidget>

static constexpr char const * toolsStr =
        "stroke()|书写|;"
        "eraser()|擦除|;";

InkPadPlugin::InkPadPlugin(QObject * parent)
    : PageBoxPlugin(parent)
{
    inkCanvas_ = InkStrokeControl::createInkCanvas(4);
    QGraphicsProxyWidget * proxy = new QGraphicsProxyWidget;
    proxy->setWidget(inkCanvas_);
    proxy->setAcceptTouchEvents(true);
    item_ = proxy;
    setToolsString(toolsStr);
}

void InkPadPlugin::stroke()
{
    if (inkCanvas_->EditingMode() == InkCanvasEditingMode::Ink)
        inkCanvas_->SetEditingMode(InkCanvasEditingMode::None);
    else
        inkCanvas_->SetEditingMode(InkCanvasEditingMode::Ink);
}

void InkPadPlugin::eraser()
{
    if (inkCanvas_->EditingMode() == InkCanvasEditingMode::EraseByStroke)
        inkCanvas_->SetEditingMode(InkCanvasEditingMode::None);
    else
        inkCanvas_->SetEditingMode(InkCanvasEditingMode::EraseByStroke);
}

void InkPadPlugin::onRelayout(int pageCount, int curPage)
{
    pageStrokes_.clear();
    pageStrokes_.resize(pageCount);
    onPageChanged(-1, curPage);
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

void InkPadPlugin::onSizeChanged(const QSizeF &docSize, const QSizeF &pageSize, const QSizeF &viewSize)
{
    qreal sw = viewSize.width() / pageSize.width();
    qreal sh = viewSize.height() / pageSize.height();
    qreal s = qMin(sh, sw);
    item_->setScale(1 / s);
    QSizeF size(docSize.width() / pageSize.width() * viewSize.width(),
                 docSize.height() / pageSize.height() * viewSize.height());
    static_cast<QGraphicsProxyWidget*>(item_)->resize(size);
}

bool InkPadPlugin::selectTest(const QPointF &pt)
{
    return InkStrokeControl::selectTest(inkCanvas_, pt) != Control::NotSelect;
}

