#include "pageboxpageitem.h"
#include "pageboxdocitem.h"

#include <core/resourceview.h>
#include <core/resource.h>

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

static void del_res(int * p) {
    delete reinterpret_cast<ResourceView *>(p);
}

void PageBoxPageItem::setImage(const QUrl &image)
{
    if (image.isEmpty())
        return;
    Resource * res = new Resource(nullptr, image);
    lifeToken_.reset(reinterpret_cast<int*>(res), del_res);
    QWeakPointer<int> life(lifeToken_);
    res->getData().then([this, life](QByteArray data) {
        if (life.isNull())
            return;
        QPixmap pixmap;
        pixmap.loadFromData(data);
        setPixmap(pixmap);
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
