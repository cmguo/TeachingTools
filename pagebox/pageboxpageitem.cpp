#include "pageboxpageitem.h"
#include "pageboxdocitem.h"

#include <data/imagecache.h>
#include <core/resource.h>
#include <qeventbus.h>

PageBoxPageItem::PageBoxPageItem(QGraphicsItem * parent)
    : QGraphicsPixmapItem(parent)
    , lifeToken_(nullptr)
{
    setShapeMode(QGraphicsPixmapItem::BoundingRectShape);
    setTransformationMode(Qt::SmoothTransformation);
    /*
    int N = 4;
    QRectF rect = {0, 0, 1.0 / N, 1.0 / N};
    for (int i = 0; i < N; ++i) {
        for (int j = 0; j < N; ++j) {
            rect.moveTo(qreal(i) / N, qreal(j) / N);
            rects_.append(rect);
        }
    }*/
}

void PageBoxPageItem::setImage(const QUrl &image)
{
    if (image.isEmpty()) {
        if (lifeToken_ && !lifeToken_->property("PageBoxPageItem").isNull())
            lifeToken_.reset();
        imageData_.reset();
        return;
    }
    if (lifeToken_.isNull() || !lifeToken_->property("PageBoxPageItem").isNull())
        lifeToken_.reset(new Resource("image", QUrl()));
    lifeToken_->setProperty("PageBoxPageItem", QVariant::fromValue(this));
    QWeakPointer<Resource> life(lifeToken_);
    ImageCache::instance().getOrCreate(lifeToken_.get(), image).then(
                [this, life](QSharedPointer<ImageData> const & data) {
        if (life.isNull())
            return;
        imageData_ = data;
        setPixmap(data->pixmap());
    }, [](std::exception & e) {
        qWarning() << "PageBoxPageItem::setImage" << e.what();
        QByteArray msg = e.what();
        msg.replace(0, msg.indexOf('|') + 1, "");
        QEventBus::globalInstance().publish("warning", msg);
    }).finally([life] () {
        if (!life.isNull())
            life.data()->setProperty("PageBoxPageItem", QVariant());
    });
}

void PageBoxPageItem::setPixmap(const QPixmap &pixmap)
{
    QGraphicsPixmapItem::setPixmap(pixmap);
    QSizeF pageSize = static_cast<PageBoxDocItem*>(parentItem()->parentItem())->pageSize();
    if (pageSize.toSize() != pixmap.size()) {
        QSizeF imageSize(pixmap.size());
        setTransform(QTransform::fromScale(
                         pageSize.width() / imageSize.width(), pageSize.height() / imageSize.height()));
    } else {
        setTransform(QTransform());
    }
}

QRectF PageBoxPageItem::hit(QPointF const & point)
{
    QRectF rc(boundingRect());
    QPointF pt(point.x() / rc.width(), point.y() / rc.height());
    for (QRectF const & rect : rects_) {
        if (rect.contains(pt))
            return QRectF(rect.left() * rc.width(), rect.top() * rc.height(),
                          rect.width() * rc.width(), rect.height() * rc.height());
    }
    return QRectF();
}
