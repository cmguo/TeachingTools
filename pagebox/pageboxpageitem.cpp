#include "pageboxpageitem.h"
#include "pageboxdocitem.h"

#include <core/resourceview.h>
#include <core/resource.h>
#include <core/imagecache.h>

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

static void nop(int *) {}

void PageBoxPageItem::setImage(const QUrl &image)
{
    if (image.isEmpty()) {
        lifeToken_.reset();
        return;
    }
    lifeToken_.reset(reinterpret_cast<int*>(1), nop);
    QWeakPointer<int> life(lifeToken_);
    ImageCache::instance().getOrCreate(image).then(
                [this, life](QSharedPointer<ImageData> const & data) {
        if (life.isNull())
            return;
        setPixmap(data->pixmap());
    }, [](std::exception & e) {
        qDebug() << e.what();
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
