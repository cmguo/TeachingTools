#include "inkpadplugin.h"
#include "pageboxdocitem.h"
#include "inkcanvas/inkstrokecontrol.h"

#include <Windows/Controls/inkcanvas.h>
#include <Windows/Controls/inkevents.h>

#include <QGraphicsProxyWidget>

InkPadPlugin::InkPadPlugin(QObject * parent)
    : PageBoxPlugin(parent)
{
    inkCanvas_ = InkStrokeControl::createInkCanvas(8);
    inkCanvas_->AddHandler(InkCanvas::StrokeCollectedEvent, RoutedEventHandlerT<
                    InkPadPlugin, InkCanvasStrokeCollectedEventArgs, &InkPadPlugin::onStrokeCollected>(this));
    QGraphicsProxyWidget * proxy = new QGraphicsProxyWidget;
    proxy->setWidget(inkCanvas_);
    item_ = proxy;
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
    QSizeF size2 = viewSize / s;
    QSizeF size3(docSize.width() / pageSize.width() * size2.width(),
                 docSize.height() / pageSize.height() * size2.height());
    static_cast<QGraphicsProxyWidget*>(item_)->resize(size3);
    size2 = (size3 - docSize) / 2;
    item_->setPos(-size2.width(), -size2.height());
}

void InkPadPlugin::onStrokeCollected(InkCanvasStrokeCollectedEventArgs &e)
{
    InkStrokeControl::applyPressure(e);
}

