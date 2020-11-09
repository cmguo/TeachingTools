#include "pageboxdocitem.h"
#include "pageboxpageitem.h"
#include "pageboxplugin.h"
#include "qpropertybindings.h"
#include "pagenumberwidget.h"
#include "qpropertybinding.h"
#include "pageanimcanvas.h"

#include <guidehelper.h>

#include <data/resourcecache.h>
#include <core/resourcerecord.h>
#include <core/toolbutton.h>
#include <views/pageswitchevent.h>

#include <Windows/Controls/inkcanvas.h>
#include <Windows/Controls/inkevents.h>

#include <QColor>
#include <QPen>
#include <QAbstractItemModel>
#include <QGraphicsScene>
#include <QGraphicsItem>
#include <QGraphicsProxyWidget>

#define DUPLEX_FIX_SIZE 1

PageBoxDocItem::PageBoxDocItem(QGraphicsItem * parent)
    : QGraphicsRectItem(parent)
    , model_(nullptr)
    , direction_(Vertical)
    , layoutMode_(Continuous)
    , padding_(0)
    , curPage_(-2)
    , resCache_(nullptr)
{
    setPen(QPen(Qt::NoPen));

    pageCanvas_ = new QGraphicsRectItem(this);
    pageCanvas_->setPen(QPen(Qt::NoPen));

    pageNumber_ = new PageNumberWidget();
    QObject::connect(pageNumber_, &PageNumberWidget::pageNumberChanged,
                     this,[=](int page){
        gotoNeighborPage(page, true);
    });
}

PageBoxDocItem::~PageBoxDocItem()
{
    for (PageBoxPlugin * plugin : plugins_)
        plugin->item()->setParentItem(nullptr);
    plugins_.clear();
    delete pageNumber_;
    for (auto page : pageCache_)
        delete page;
    pageCache_.clear();
}

void PageBoxDocItem::setPageSize(QSizeF const & size)
{
    pageSize_ = size;
    relayout();
}

void PageBoxDocItem::setDirection(Direction direction)
{
    if (direction_ == direction)
        return;
    direction_ = direction;
    if (layoutMode_ != Single)
        relayout();
}

void PageBoxDocItem::setLayoutMode(LayoutMode mode)
{
    if (layoutMode_ == mode)
        return;
    RecordMergeScope rs(this);
    if (rs && rs.atTop())
        rs.add(MakeFunctionRecord(
                [this, mode = layoutMode_] { setLayoutMode(mode); },
                [this, mode] { setLayoutMode(mode); }
        ));
    layoutMode_ = mode;
    emit layoutModeChanged();
    relayout();
}

void PageBoxDocItem::setPadding(qreal pad)
{
    padding_ = pad;
}

void PageBoxDocItem::setBorderSize(const QRectF &border)
{
    borderSize_ = border;
}

QSizeF PageBoxDocItem::documentSize() const
{
    return rect().size();
}

int PageBoxDocItem::pageCount() const
{
    return model_ != nullptr ? model_->rowCount() : 0;
}

void PageBoxDocItem::addPlugin(PageBoxPlugin *plugin)
{
    QGraphicsItem* pluginItem = plugin->item();
    pluginItem->setParentItem(this);
    if (model_ && !pageSize2_.isEmpty()) {
        plugin->onSizeChanged(rect().size(), pageSize2_);
        plugin->onRelayout(pageCount());
        plugin->onPageChanged(-1, curPage_);
    }
    connect(plugin, &PageBoxPlugin::buttonsChanged,
            this, &PageBoxDocItem::buttonsChanged);
    plugins_.append(plugin);
    buttonsChanged();
}

void PageBoxDocItem::removePlugin(PageBoxPlugin *plugin)
{
    int index = plugins_.indexOf(plugin);
    if (index < 0)
        return;
    plugin->disconnect(this);
    if (scene())
        scene()->removeItem(plugin->item());
    else
        plugin->item()->setParentItem(nullptr);
    plugins_.removeAt(index);
    buttonsChanged();
}

qreal PageBoxDocItem::requestScale(const QSizeF &boxSize, bool whole)
{
    if (whole) {
        qreal sw = boxSize.width() / pageSize2_.width();
        qreal sh = boxSize.height() / pageSize2_.height();
        return qMin(sh, sw);
    } else if (direction_ == Vertical) { // FitLayout
        return boxSize.width() / pageSize2_.width();
    } else {
        return boxSize.height() / pageSize2_.height();
    }
}

void PageBoxDocItem::visiblePositionHint(QGraphicsItem * from, const QPointF &pos)
{
    QPointF off = mapFromItem(from, pos);
    onVisibleCenterChanged(off);
}

void PageBoxDocItem::setInitialPage(int page)
{
    if (curPage_ == -2)
        curPage_ = -3 - page;
}

void PageBoxDocItem::setItems(QAbstractItemModel * model)
{
    if (model_ == model)
        return;
    if (model_ != nullptr) {
        model_->disconnect(this);
    }
    model_ = model;
    if (model_ != nullptr) {
        QObject::connect(model_, &QAbstractItemModel::rowsInserted,
                         this, &PageBoxDocItem::resourceInserted);
        QObject::connect(model_, &QAbstractItemModel::rowsRemoved,
                         this, &PageBoxDocItem::resourceRemoved);
        QObject::connect(model_, &QAbstractItemModel::rowsMoved,
                         this, &PageBoxDocItem::resourceMoved);
    }
    int n = model_ ? model_->rowCount() : 0;
    pageNumber_->setTotal(n);
    emit pageCountChanged(n);
    relayout();
}

void PageBoxDocItem::setItemBindings(QPropertyBindings * bindings)
{
    itemBindings_ = bindings;
}

void PageBoxDocItem::reset()
{
    clear();
    itemBindings_ = nullptr;
    model_ = nullptr;
    pageSize_ = pageSize2_ = QSizeF();
    curPage_ = -2;
}

void PageBoxDocItem::resetCurrent()
{
    if (layoutMode_ == Single && curPage_ >= 0) {
        PageBoxPageItem * pageItem = static_cast<PageBoxPageItem *>(pageCanvas_->childItems().front());
        setPageImage(pageItem);
        int lastPage = curPage_;
        curPage_ = -1;
        onCurrentPageChanged(lastPage, curPage_);
    }
}

void PageBoxDocItem::setResourceCache(ResourceCache *cache)
{
    resCache_ = cache;
}

void PageBoxDocItem::clear()
{
    for(QGraphicsItem * item : pageCanvas_->childItems()) {
        if (scene())
            scene()->removeItem(item);
        PageBoxPageItem * page = static_cast<PageBoxPageItem*>(item);
        setPageImage(page);
        pageCache_.prepend(page);
    }
}

PageBoxPageItem *PageBoxDocItem::allocPage(QGraphicsItem * canvas)
{
    if (pageCache_.isEmpty())
        return new PageBoxPageItem(canvas);
    PageBoxPageItem * page = pageCache_.takeFirst();
    page->setParentItem(canvas);
    return page;
}

void PageBoxDocItem::relayout()
{
    if (pageSize_.isEmpty() || !model_)
        return;
    clear();
    int oldPage = curPage_;
    if (curPage_ == -2)
        curPage_ = initialPage();
    else if (curPage_ < -2)
        curPage_ = -curPage_ - 3;
    if (layoutMode_ == Duplex
            && curPage_ != 0
            && curPage_ % 2 == 1
            && curPage_ + 1 < model_->rowCount())
        ++curPage_;
    QRectF rect = layoutPage(pageCanvas_, curPage_);
    pageSize2_ = rect.size();
    setRect({QPointF(), rect.topLeft()});
    onPageSize2Changed(pageSize2_);
    // Initial position; TODO:
    if (oldPage < -1) {
        QPointF pos = -borderSize_.topLeft();
        pos.setX(-100000001.0); // not changed x pos
        emit requestPosition(pos);
    }
    for (PageBoxPlugin * plugin : plugins_) {
        if (oldPage < 0)
            plugin->onRelayout(pageCount());
    }
    onCurrentPageChanged(oldPage, curPage_);
}

int PageBoxDocItem::initialPage()
{
    return model_->rowCount() > 0 ? 0 : -1;
}

void PageBoxDocItem::onPageSize2Changed(const QSizeF &size)
{
    pageSize2Changed(size);
    for (PageBoxPlugin * plugin : plugins_)
        plugin->onSizeChanged(rect().size(), size);
}

void PageBoxDocItem::onVisibleCenterChanged(const QPointF &pos)
{
    if (layoutMode_ != Continuous)
        return;
    int lastPage = curPage_;
    if (direction_ == Vertical)
        curPage_ = static_cast<int>(pos.y() / (pageSize_.height() + padding()));
    else
        curPage_ = static_cast<int>(pos.x() / (pageSize_.width() + padding()));
    if (lastPage != curPage_) {
        onCurrentPageChanged(lastPage, curPage_);
    }
}

void PageBoxDocItem::onCurrentPageChanged(int last, int cur)
{
    for (PageBoxPlugin * plugin : plugins_) {
        plugin->onPageChanged(last, cur);
    }
    if (resCache_) {
        QList<QUrl> list;
        QPropertyBinding * binding = itemBindings_->getBinding("image");
        int n = 6;
        for (int i = cur + 1; n > 0 && i < pageCount(); ++i, --n) {
            QVariant item = model_->data(model_->index(i, 0), Qt::UserRole + 1);
            list.append(binding->value(item).value<QUrl>());
        }
        resCache_->reset(list);
    }
    pageNumber_->setNumber(cur);
    emit currentPageChanged(cur);
}

QPixmap PageBoxDocItem::defaultImage(int item)
{
    static QPixmap loading1(":teachingtools/image/page1.png");
    static QPixmap loading2(":teachingtools/image/page2.png");
    return item == 0 ? loading1 : loading2;
}

bool PageBoxDocItem::hit(QPointF const & point)
{
    for(QGraphicsItem * item : pageCanvas_->childItems()) {
        QPointF pt(mapToItem(item, point));
        if (item->contains(pt)) {
            QRectF rect = static_cast<PageBoxPageItem*>(item)->hit(pt);
            if (rect.isEmpty())
                continue;
            /*
            selPage_ = 0;
            selRect_ = rect;
            emit selectedChange(selPage_, selRect_);
            static_cast<SelectBox *>(selBox_)->setRect(
                        mapFromItem(item, rect).boundingRect());
            selBox_->setVisible(true);
            */
            return true;
        }
    }
    /*
    selPage_ = -1;
    selBox_->setVisible(false);
    emit selectedChange(selPage_, selRect_);
    */
    return false;
}

void PageBoxDocItem::nextPage()
{
    if (layoutMode_ == Duplex && curPage_ != 0)
        gotoNeighborPage(curPage_ + 2);
    else
        gotoNeighborPage(curPage_ + 1);
}

void PageBoxDocItem::previousPage()
{
    if (layoutMode_ == Duplex && curPage_ > 2)
        gotoNeighborPage(curPage_ - 2);
    else
        gotoNeighborPage(curPage_ - 1);
}

void PageBoxDocItem::frontPage()
{
    goToPage(0);
}

void PageBoxDocItem::backPage()
{
    goToPage(model_->rowCount() - 1);
}

void PageBoxDocItem::goToPage(int page, bool anim)
{
    page = adjustPageIndex(page, false);
    switchPage(page, anim);
}

void PageBoxDocItem::gotoNeighborPage(int page, bool anim)
{
    page = adjustPageIndex(page, true);
    switchPage(page, anim);
}

int PageBoxDocItem::adjustPageIndex(int page, bool neighbor)
{
    if (layoutMode_ == Duplex && page != 0 && page % 2 == 1
            && page + 1 < model_->rowCount()) {
        if (neighbor)
            page = (page == curPage_ - 1) ? page - 1 : page + 1;
        else
            ++page;
    }
    if (page < 0 || page >= model_->rowCount() || page == curPage_)
        page = -1;
    return page;
}

void PageBoxDocItem::switchPage(int page, bool anim)
{
    bool guided = GuideHelper::sendGuideEvent(GestureType::TurnPage);
    if (guided) return;
    if (!model_ || page == curPage_)
        return;
    if (page < 0) {
        onCurrentPageChanged(curPage_, curPage_);
        return;
    }
    if (layoutMode_ == Continuous) {
        QPointF off(-100000001.0, -100000001.0);
        if (direction_ == Vertical)
            off.setY((pageSize_.height() + padding_) * page);
        else
            off.setX((pageSize_.width() + padding_) * page);
        emit requestPosition(off);
    } else {
        if (anim) {
            createAnimCanvas(page, true);
            animCanvas_->startAnimate();
        } else {
            clear();
            QRectF rect = layoutPage(pageCanvas_, page);
#if DUPLEX_FIX_SIZE
            (void) rect;
#else
            if (pageSize2_ != rect.size()) {
                setRect(QRectF(QPointF(0, 0), rect.topLeft()));
                pageSize2_ = size;
                onSizeChanged(pageSize2_);
                rescale();
            }
#endif
        }
    }
    int lastPage = curPage_;
    curPage_ = page;
    onCurrentPageChanged(lastPage, curPage_);
}

QByteArray PageBoxDocItem::saveState()
{
    if (resCache_)
        resCache_->moveBackground();
    char * p1 = reinterpret_cast<char *>(&pageSize_);
    char * p2 = reinterpret_cast<char *>(&pageCanvas_);
    QByteArray data(p1, static_cast<int>(p2 - p1));
    return data;
}

void PageBoxDocItem::restoreState(QByteArray data)
{
    char * p1 = reinterpret_cast<char *>(&pageSize_);
    char * p2 = reinterpret_cast<char *>(&pageCanvas_);
    memcpy(p1, data.data(), static_cast<size_t>(p2 - p1));
    if (resCache_)
        resCache_->moveFront();
}

void PageBoxDocItem::getToolButtons(QList<ToolButton *> &buttons, const QList<ToolButton *> &parents)
{
    ToolButtonProvider::getToolButtons(buttons, parents);
    for (PageBoxPlugin * plugin : plugins_) {
        plugin->getToolButtons(buttons, parents);
    }
}

bool PageBoxDocItem::handleToolButton(const QList<ToolButton *> &buttons)
{
    if (ToolButtonProvider::handleToolButton(buttons))
        return true;
    for (PageBoxPlugin * plugin : plugins_) {
        if (plugin->handleToolButton(buttons))
            return true;
    }
    return false;
}

void PageBoxDocItem::getToolButtons(QList<ToolButton *> &buttons, ToolButton *parent)
{
    ToolButtonProvider::getToolButtons(buttons, parent);
    if (parent == nullptr) {
        for (ToolButton * & b : buttons) {
            if (b->name() == "pages")
                b = pageNumber_->toolButton();
        }
    }
}

void PageBoxDocItem::resourceInserted(QModelIndex const &parent, int first, int last)
{
    (void) parent;
    (void) first;
    (void) last;
}

void PageBoxDocItem::resourceRemoved(QModelIndex const &parent, int first, int last)
{
    (void) parent;
    (void) first;
    (void) last;
}

void PageBoxDocItem::resourceMoved(QModelIndex const &parent, int start, int end,
                   QModelIndex const &destination, int row)
{
    (void) parent;
    (void) start;
    (void) end;
    (void) destination;
    (void) row;
}

QRectF PageBoxDocItem::layoutPage(QGraphicsItem *canvas, int page)
{
    QPointF pos = -borderSize_.topLeft();
    QPointF off; // offset of one page, with padding
    QSizeF pageSize2 = pageSize_;
    if (direction_ == Vertical) {
        pos.setY(padding_);
        off.setY(pageSize_.height() + padding_);
        pageSize2.setHeight(pageSize_.height() + padding_ * 2);
    } else {
        pos.setX(padding_);
        off.setX(pageSize_.width() + padding_);
        pageSize2.setWidth(pageSize_.width() + padding_ * 2);
    }
    if (layoutMode_ == Single || layoutMode_ == DuplexSingle) {
        PageBoxPageItem * pageItem = allocPage(canvas);
        setPageImage(pageItem, page);
        pageItem->setPos(pos);
        pos += off;
        // special case, transform attach prevent us to place single page at center
        if (layoutMode_ == DuplexSingle) {
            pageItem->setPos(pos - off / 2);
            pos += off;
            pageSize2 += QSizeF(off.x(), off.y());
        }
    } else if (layoutMode_ == Duplex) {
        PageBoxPageItem * pageItem1 = allocPage(canvas);
        pageItem1->setPos(pos);
        pos += off;
        setPageImage(pageItem1, (page == 0 || (page & 1)) ? page : page - 1);
        if (page && (page & 1) == 0) {
            PageBoxPageItem * pageItem2 = allocPage(canvas);
            pageItem2->setPos(pos);
            pos += off;
            setPageImage(pageItem2, page, true);
            pageSize2 += QSizeF(off.x(), off.y());
        } else {
#if DUPLEX_FIX_SIZE
            pageSize2 += QSizeF(off.x(), off.y());
            pos += off;
            pageItem1->setPos(pageItem1->pos() + off / 2);
#endif
        }
    } else {
        for (int i = 0; i < model_->rowCount(); ++i) {
            PageBoxPageItem * pageItem = allocPage(canvas);
            setPageImage(pageItem, i);
            pageItem->setPos(pos);
            pos += off;
        }
    }
    if (direction_ == Vertical) {
        pos.setX(pos.x() + pageSize_.width());
    } else {
        pos.setY(pos.y() + pageSize_.height());
    }
    pos += borderSize_.bottomRight();
    return {pos, pageSize2};
}

void PageBoxDocItem::setPageImage(PageBoxPageItem *pageItem, int index, bool second)
{
    if (pageItem->parentItem())
        pageItem->setPixmap(defaultImage(second ? 1 : 0));
    if (index >= 0) {
        QVariant item1 = model_->data(model_->index(index, 0), Qt::UserRole + 1);
        itemBindings_->bind(QVariant::fromValue(pageItem), item1);
    } else {
        itemBindings_->unbind(QVariant::fromValue(pageItem));
        pageItem->setImage(QUrl());
    }
}

bool PageBoxDocItem::createAnimCanvas(int page, bool afterPageSwitch)
{
    if (animCanvas_) {
        if (!afterPageSwitch && (animCanvas_->afterPageSwitch()
                || animCanvas_->inAnimate()))
            return false;
        animCanvas_->stopAnimate(); // will clear animCanvas_
    }
    animCanvas_ = new PageAnimCanvas(this);
    animCanvas_->setPen(QPen(Qt::NoPen));
    animCanvas_->setAfterPageSwitch(afterPageSwitch);
    animCanvas_->setDirection(page > curPage_
                              ? PageAnimCanvas::RightToLeft
                              : PageAnimCanvas::LeftToRight);
    layoutPage(animCanvas_, page);
    if (!afterPageSwitch)
        animCanvas_->setData(1000, page);
    QRectF vrect = mapFromScene(scene()->sceneRect()).boundingRect();
    if (page > curPage_)
        animCanvas_->setY(vrect.top() + borderSize_.top());
    else
        animCanvas_->setY(vrect.bottom() + borderSize_.top() - pageSize2_.height());
    qDebug() << "PageBoxDocItem::createAnimCanvas" << animCanvas_->pos();
    return true;
}

void PageBoxDocItem::destroyAnimCanvas(PageAnimCanvas * anim, bool finish)
{
    assert(animCanvas_ == anim);
    if (finish && animCanvas_->switchPage()) {
        QRectF vrect = mapFromScene(scene()->sceneRect()).boundingRect();
        qreal width1 = (pageCanvas_->childItems().size() == 1
                       ? pageSize_ : pageSize2_).width();
        qreal width2 = pageSize2_.width();
        QPointF pos = -borderSize_.topLeft();
        if (animCanvas_->direction() == PageAnimCanvas::LeftToRight)
            pos += QPointF((width1 + width2) / 2 - vrect.width(),
                           pageSize2_.height() - vrect.height());
        else
            pos += QPointF((width2 - width1) / 2, 0);
        if (vrect.width() > width1)
            pos.setX((rect().width() - vrect.width()) / 2);
        qDebug() << "PageBoxDocItem::destroyAnimCanvas" << pos;
        requestPosition(pos);
    }
    delete animCanvas_;
    animCanvas_ = nullptr;
}

bool PageBoxDocItem::event(QEvent *event)
{
    switch (event->type()) {
    case PageSwitchEvent::PageSwitchStart: {
        int page = (static_cast<PageSwitchStartEvent*>(event)
                ->delta().x() < 0 ? 1 : -1) + curPage_;
        page = adjustPageIndex(page, true);
        if (page < 0)
            break;
        if (createAnimCanvas(page, false)) {
            animCanvas_->startAnimate();
            event->setAccepted(true);
        }
    }
        break;
    case PageSwitchEvent::PageSwitchMove:
        event->setAccepted(animCanvas_->move(
                               static_cast<PageSwitchMoveEvent*>(event)->delta()));
        break;
    case PageSwitchEvent::PageSwitchEnd:
        if (animCanvas_ && animCanvas_->release()) {
            if (!animCanvas_->afterPageSwitch()) {
                int lastPage = curPage_;
                curPage_ = animCanvas_->data(1000).toInt();
                onCurrentPageChanged(lastPage, curPage_);
            }
            event->accept();
        }
        break;
    default:
        return QObject::event(event);
    }
    return event->isAccepted();
}

