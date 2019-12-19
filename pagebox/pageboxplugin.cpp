#include "pageboxplugin.h"
#include "pageboxdocitem.h"

#include <QGraphicsItem>

PageBoxPlugin::PageBoxPlugin()
    : item_(nullptr)
{
}

PageBoxPlugin::~PageBoxPlugin()
{
    if (item_)
        delete item_;
}

void PageBoxPlugin::onRelayout(int pageCount, int curPage)
{
    (void) pageCount;
    (void) curPage;
}

void PageBoxPlugin::onPageChanged(int lastPage, int curPage)
{
    (void) lastPage;
    (void) curPage;
}

void PageBoxPlugin::onSizeChanged(const QSizeF &docSize, const QSizeF &pageSize,
                                const QSizeF &viewSize)
{
    (void) docSize;
    (void) pageSize;
    (void) viewSize;
}

PageBoxDocItem *PageBoxPlugin::document()
{
    return static_cast<PageBoxDocItem*>(item_->parentItem());
}
