#include "pageboxdocitem.h"
#include "pageboxpageitem.h"
#include "pageboxitem.h"
#include "pageboxplugin.h"
#include "qpropertybindings.h"

#include <core/controltransform.h>
#include <core/resourcetransform.h>

#include <Windows/Controls/inkcanvas.h>
#include <Windows/Controls/inkevents.h>

#include <QColor>
#include <QPen>
#include <QAbstractItemModel>
#include <QGraphicsScene>
#include <QGraphicsItem>
#include <QGraphicsProxyWidget>

#include <cmath>

#ifdef QT_DEBUG
#define DUPLEX_FIX_SIZE 1
#else
#define DUPLEX_FIX_SIZE 0
#endif

PageBoxDocItem::PageBoxDocItem(QGraphicsItem * parent)
    : QGraphicsRectItem(parent)
    , model_(nullptr)
    , transform_(nullptr)
    , direction_(Vertical)
    , layoutMode_(Continuous)
    , scaleMode_(FitLayout)
    , padding_(0)
    , curPage_(-1)
    , scaleInterval_(1.2)
    , scaleLevel_(0)
    , maxScaleLevel_(0)
    , plugin_(nullptr)
    , pluginItem_(nullptr)
{
    pageCanvas_ = new QGraphicsRectItem(this);
    pageCanvas_->setPen(QPen(Qt::NoPen));

    setPen(QPen(Qt::NoPen));

    transform_ = new ResourceTransform(this);
    setTransformations({new ControlTransform(*transform_)});
    QObject::connect(transform_, &ResourceTransform::changed, this, &PageBoxDocItem::onTransformChanged);
}

void PageBoxDocItem::setPageSize(QSizeF size)
{
    pageSize_ = size;
    rescale();
    relayout();
}

void PageBoxDocItem::setDirection(Direction direction)
{
    if (direction_ == direction)
        return;
    direction_ = direction;
    if (layoutMode_ != Single)
        relayout();
    rescale();
}

void PageBoxDocItem::setLayoutMode(LayoutMode mode)
{
    if (layoutMode_ == mode)
        return;
    layoutMode_ = mode;
    emit layoutModeChanged();
    relayout();
    rescale();
}

void PageBoxDocItem::setScaleMode(ScaleMode mode)
{
    if (scaleMode_ == mode)
        return;
    scaleMode_ = mode;
    rescale();
}

void PageBoxDocItem::setManualScale(qreal scale, bool changeMode)
{
    if (scaleMode_ == ManualScale && qFuzzyIsNull(manualScale_ - scale))
        return;
    manualScale_ = scale;
    if (changeMode)
        scaleMode_ = ManualScale;
    if (scaleMode_ == ManualScale)
        rescale();
}

void PageBoxDocItem::transferToManualScale()
{
    QRectF vrect = static_cast<PageBoxItem*>(parentItem())->visibleRect();
    qreal sw = vrect.width() / pageSize1_.width();
    qreal sh = vrect.height() / pageSize1_.height();
    qreal sMin = qMin(sh, sw);
    qreal sMax = qMax(sh, sw);
    qreal d = sMax / sMin;
    maxScaleLevel_ = 1;
    while (d >= 1.44) {
        maxScaleLevel_ *= 2;
        d = sqrt(d);
    }
    scaleInterval_ = d;
    manualScale_ = scale();
    sMin *= scaleInterval_;
    while (sMin < manualScale_) {
        sMin *= scaleInterval_;
        ++scaleLevel_;
    }
    scaleMode_ = ManualScale;
}

void PageBoxDocItem::stepScale(bool up)
{
    if (up) {
        if (scaleLevel_ < maxScaleLevel_) {
            ++scaleLevel_;
            setManualScale(scale() * scaleInterval_);
        }
    } else {
        if (scaleLevel_ > 0) {
            --scaleLevel_;
            setManualScale(scale() / scaleInterval_);
        }
    }
}

bool PageBoxDocItem::canStepScale(bool up)
{
    if (up)
        return scaleLevel_ < maxScaleLevel_;
    else
        return scaleLevel_ > 0;
}

void PageBoxDocItem::stepMiddleScale()
{
    qreal s = scale();
    for (; scaleLevel_ < maxScaleLevel_ / 2; ++scaleLevel_)
        s *= scaleInterval_;
    for (; scaleLevel_ > maxScaleLevel_ / 2; --scaleLevel_)
        s /= scaleInterval_;
    setManualScale(s);
    QPointF off = transform_->offset();
    off.setY(parentItem()->boundingRect().top());
    transform_->translateTo(off);
}

void PageBoxDocItem::setPadding(qreal pad)
{
    padding_ = pad;
}

void PageBoxDocItem::setPlugin(PageBoxPlugin* plugin)
{
    if (plugin == plugin_)
        return;
    if (plugin_) {
        scene()->removeItem(pluginItem_);
    }
    plugin_ = plugin;
    if (plugin_) {
        pluginItem_ = plugin_->item();
        pluginItem_->setParentItem(this);
    } else {
        pluginItem_ = nullptr;
    }
}

QSizeF PageBoxDocItem::documentSize() const
{
    return rect().size();
}

qreal PageBoxDocItem::scale() const
{
    return transform_->scale().m11();
}

QPointF PageBoxDocItem::offset() const
{
    return QPointF(transform_->translate().dx(), transform_->translate().dy());
}

int PageBoxDocItem::pageCount() const
{
    return model_ != nullptr ? model_->rowCount() : 0;
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
    if (pos_.isNull())
        curPage_ = 0;
    emit pageCountChanged(model_ ? model_->rowCount() : 0);
    relayout();
    rescale();
    onCurrentPageChanged();
}

void PageBoxDocItem::setItemBindings(QPropertyBindings * bindings)
{
    itemBindings_ = bindings;
}

void PageBoxDocItem::moveBy(qreal dx, qreal dy)
{
    transform_->translate(QPointF(dx, dy));
}

void PageBoxDocItem::relayout()
{
    if (pageSize_.isEmpty() || !model_)
        return;
    QPointF pos;
    QPointF off;
    pageSize1_ = pageSize2_ = pageSize_;
    if (direction_ == Vertical) {
        pos.setY(padding_);
        off.setY(pageSize_.height() + padding_);
        pageSize2_.setHeight(pageSize_.height() + padding_ * 2);
    } else {
        pos.setX(padding_);
        off.setX(pageSize_.width() + padding_);
        pageSize2_.setWidth(pageSize_.width() + padding_ * 2);
    }
    for(QGraphicsItem * item : pageCanvas_->childItems()) {
        if (pluginItem_ != item) {
            scene()->removeItem(item);
            itemBindings_->unbind(QVariant::fromValue(item));
            delete item;
        }
    }
    if (layoutMode_ == Single) {
        QVariant item0 = model_->data(model_->index(curPage_, 0), Qt::UserRole + 1);
        PageBoxPageItem * pageItem = new PageBoxPageItem(pageCanvas_);
        //pageItem->stackBefore(selBox_);
        itemBindings_->bind(QVariant::fromValue(pageItem), item0);
        pageItem->setPos(pos);
        pos += off;
        // special case, transform attach prevent us to place single page at center
        if (static_cast<PageBoxItem*>(parentItem())->sizeMode() == PageBoxItem::LargeCanvas) {
            pageItem->setPos(pos - off / 2);
            pos += off;
        }
    } else if (layoutMode_ == Duplex) {
        if (layoutMode_ == Duplex && curPage_ != 0 && curPage_ % 2 == 1 && curPage_ + 1 < model_->rowCount())
            ++curPage_;
        QVariant item1 = model_->data(model_->index((curPage_ == 0 || (curPage_ & 1)) ? curPage_ : curPage_ - 1, 0), Qt::UserRole + 1);
        PageBoxPageItem * pageItem1 = new PageBoxPageItem(pageCanvas_);
        //pageItem->stackBefore(selBox_);
        itemBindings_->bind(QVariant::fromValue(pageItem1), item1);
        pageItem1->setPos(pos);
        pos += off;
        PageBoxPageItem * pageItem2 = new PageBoxPageItem(pageCanvas_);
        //pageItem2->stackBefore(selBox_);
        pageItem2->setPos(pos);
        if (curPage_) {
            pos += off;
            pageSize2_ += QSizeF(off.x(), off.y());
            if ((curPage_ & 1) == 0) {
                QVariant item2 = model_->data(model_->index(curPage_, 0), Qt::UserRole + 1);
                itemBindings_->bind(QVariant::fromValue(pageItem2), item2);
            }
        } else {
#if DUPLEX_FIX_SIZE
            pageSize2_ += QSizeF(off.x(), off.y());
            pos += off;
            pageItem1->setPos(pageItem1->pos() + off / 2);
#endif
            pageItem2->setVisible(false);
        }
    } else {
        for (int i = 0; i < model_->rowCount(); ++i) {
            QVariant item = model_->data(model_->index(i, 0), Qt::UserRole + 1);
            PageBoxPageItem * pageItem = new PageBoxPageItem(pageCanvas_);
            //pageItem->stackBefore(selBox_);
            itemBindings_->bind(QVariant::fromValue(pageItem), item);
            pageItem->setPos(pos);
            pos += off;
        }
    }
    if (plugin_) {
        if (pos_.isNull())
            plugin_->onRelayout(pageCount(), curPage_);
        else
            plugin_->onPageChanged(-1, curPage_);
    }
    onCurrentPageChanged();
    if (direction_ == Vertical) {
        pos.setX(pageSize_.width());
    } else {
        pos.setY(pageSize_.height());
    }
    setRect(QRectF(QPointF(0, 0), pos));
    onSizeChanged(pageSize2_);
}

void PageBoxDocItem::rescale()
{
    if (pageSize2_.isEmpty() || !model_)
        return;
    QRectF vrect = static_cast<PageBoxItem*>(parentItem())->visibleRect();
    qreal s = 1.0;
    if (scaleMode_ == ManualScale) {
        s = manualScale_;
    } else if (scaleMode_ == WholePage) {
        qreal sw = vrect.width() / pageSize2_.width();
        qreal sh = vrect.height() / pageSize2_.height();
        s = qMin(sh, sw);
    } else if (direction_ == Vertical) { // FitLayout
        s = vrect.width() / pageSize2_.width();
    } else {
        s = vrect.height() / pageSize2_.height();
    }
    QRectF rect = this->rect();
    transform_->scaleKeepToCenter(vrect, rect, s);
}

void PageBoxDocItem::onSizeChanged(const QSizeF &size)
{
    sizeChanged(size);
    if (plugin_)
        plugin_->onSizeChanged(rect().size(), size, parentItem()->boundingRect().size());
}

void PageBoxDocItem::onTransformChanged()
{
    if (layoutMode_ == Continuous) {
        QRectF vrect = static_cast<PageBoxItem*>(parentItem())->visibleRect();
        QPointF off = (vrect.center() - transform_->transform().map(rect().topLeft())) / scale();
        //qDebug() << "PageBoxDocItem" << transform_->transform() << vrect << off;
        int lastPage = curPage_;
        if (direction_ == Vertical)
            curPage_ = static_cast<int>(off.y() / (pageSize_.height() + padding()));
        else
            curPage_ = static_cast<int>(off.x() / (pageSize_.width() + padding()));
        if (lastPage != curPage_) {
            if (plugin_)
                plugin_->onPageChanged(lastPage, curPage_);
            onCurrentPageChanged();
        }
    }
}

void PageBoxDocItem::onCurrentPageChanged()
{
    emit currentPageChanged(curPage_);
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

ResourceTransform * PageBoxDocItem::detachTransform()
{
    //QObject::disconnect(transform_, &ResourceTransform::changed, this, &PageBoxDocItem::onTransformChanged);
    //transform_ = new ResourceTransform(*transform_, this);
    //QObject::connect(transform_, &ResourceTransform::changed, this, &PageBoxDocItem::onTransformChanged);
    if (!transformations().empty()) { // else already detach in restore
        setTransform(transform_->transform());
        setTransformations({});
    }
    return transform_;
}

void PageBoxDocItem::nextPage()
{
    if (layoutMode_ == Duplex && curPage_ != 0)
        goToPage(curPage_ + 2);
    else
        goToPage(curPage_ + 1);
}

void PageBoxDocItem::previousPage()
{
    if (layoutMode_ == Duplex && curPage_ > 2)
        goToPage(curPage_ - 2);
    else
        goToPage(curPage_ - 1);
}

void PageBoxDocItem::frontPage()
{
    goToPage(0);
}

void PageBoxDocItem::backPage()
{
    goToPage(model_->rowCount() - 1);
}

void PageBoxDocItem::goToPage(int page)
{
    if (!model_ || page == curPage_)
        return;
    if (layoutMode_ == Duplex && page != 0 && page % 2 == 1) {
        page = (page == curPage_ - 1) ? page - 1 : page + 1;
        if (page == model_->rowCount())
            --page;
    }
    if (page < 0 || page >= model_->rowCount() || page == curPage_) { // after adjust
        onCurrentPageChanged();
        return;
    }
    int lastPage = curPage_;
    curPage_ = page;
    if (plugin_) {
        plugin_->onPageChanged(lastPage, curPage_);
    }
    static QPixmap loading1(":teachingtools/image/page1.png");
    static QPixmap loading2(":teachingtools/image/page2.png");
    if (layoutMode_ == Single) {
        QVariant item = model_->data(model_->index(page, 0), Qt::UserRole + 1);
        PageBoxPageItem * pageItem = static_cast<PageBoxPageItem *>(pageCanvas_->childItems().front());
        pageItem->setPixmap(loading1);
        itemBindings_->bind(QVariant::fromValue(pageItem), item);
    } else if (layoutMode_ == Duplex) {
        QVariant item1 = model_->data(model_->index((curPage_ == 0 || (curPage_ & 1)) ? curPage_ : curPage_ - 1, 0), Qt::UserRole + 1);
        PageBoxPageItem * pageItem1 = static_cast<PageBoxPageItem *>(pageCanvas_->childItems()[0]);
        PageBoxPageItem * pageItem2 = static_cast<PageBoxPageItem *>(pageCanvas_->childItems()[1]);
        pageItem1->setPixmap(loading1);
        pageItem2->setPixmap(loading2);
        QSizeF off;
        if (direction_ == Vertical) {
            off.setHeight(pageSize_.height() + padding_);
        } else {
            off.setWidth(pageSize_.width() + padding_);
        }
        itemBindings_->bind(QVariant::fromValue(pageItem1), item1);
#if !DUPLEX_FIX_SIZE
        QSizeF& size = pageSize2_;
#endif
        if (curPage_ == 0) {
#if DUPLEX_FIX_SIZE
            pageItem1->setPos(pageItem1->pos() + QPointF(off.width(), off.height()) / 2);
#else
            size -= off;
#endif
            pageItem2->setVisible(false);
        } else {
            if (lastPage == 0)
#if DUPLEX_FIX_SIZE
                pageItem1->setPos(pageItem1->pos() - QPointF(off.width(), off.height()) / 2);
#else
                size += off;
#endif
            pageItem2->setVisible(true);
            if ((curPage_ & 1) == 0) {
                QVariant item2 = model_->data(model_->index(curPage_, 0), Qt::UserRole + 1);
                itemBindings_->bind(QVariant::fromValue(pageItem2), item2);
            }
        }
#if !DUPLEX_FIX_SIZE
        setRect(QRectF(QPointF(0, 0), size));
        onSizeChanged(pageSize2_);
        rescale();
#endif
    } else {
        QPointF topLeft = parentItem()->boundingRect().topLeft();
        QPointF off(offset());
        if (direction_ == Vertical)
            off.setY(topLeft.y() + (pageSize_.height() + padding_) * -curPage_ * scale());
        else
            off.setX(topLeft.x() + (pageSize_.width() + padding_) * -curPage_ * scale());
        qDebug() << "goToPage" << page << off;
        transform_->translateTo(off);
    }
    onCurrentPageChanged();
}

struct TransformData
{
    TransformData() {}
    TransformData(QTransform const & t)
    {
        scale = t.m11();
        offset.setX(t.dx());
        offset.setY(t.dy());
    }
    QTransform toQTransform() const
    {
        return QTransform::fromTranslate(offset.x(), offset.y()).scale(scale, scale);
    }
    qreal scale;
    QPointF offset;
};

QByteArray PageBoxDocItem::saveState()
{
    qDebug() << "PageBoxDocItem saveState" << transform_->transform();
    pos_ = offset();
    char * p1 = reinterpret_cast<char *>(&pageSize_);
    char * p2 = reinterpret_cast<char *>(&pageCanvas_);
    QByteArray data(p1, static_cast<int>(p2 - p1));
    if (transformations().empty()) {
        TransformData d(transform());
        data.append(reinterpret_cast<char *>(&d), sizeof(d));
    }
    return data;
}

void PageBoxDocItem::restoreState(QByteArray data)
{
    char * p1 = reinterpret_cast<char *>(&pageSize_);
    char * p2 = reinterpret_cast<char *>(&pageCanvas_);
    memcpy(p1, data.data(), static_cast<size_t>(p2 - p1));
    if (data.size() > p2 - p1) {
        TransformData d;
        memcpy(reinterpret_cast<char *>(&d), data.data() + (p2 - p1), sizeof(d));
        setTransform(d.toQTransform());
        setTransformations({});
    }
}

void PageBoxDocItem::restorePosition()
{
    if (!pos_.isNull()) {
        transform_->translateTo(pos_);
        pos_ = QPointF();
        qDebug() << "PageBoxDocItem restorePosition" << transform_->transform();
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

