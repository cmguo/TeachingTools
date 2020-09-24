#ifndef PAGEBOXPAGEITEM_H
#define PAGEBOXPAGEITEM_H

#include <QGraphicsPixmapItem>
#include <QUrl>
#include <QList>

class Resource;
class ImageData;

class PageBoxPageItem : public QGraphicsPixmapItem
{
    Q_GADGET

    Q_PROPERTY(QUrl image READ image WRITE setImage)
    Q_PROPERTY(QList<QRectF> rects MEMBER rects_)

public:
    PageBoxPageItem(QGraphicsItem * parent = nullptr);

public:
    QRectF hit(QPointF const & point);

    QUrl image() const
    {
        return image_;
    }

    void setImage(QUrl const & image);

public:
    void setPixmap(QPixmap const & pixmap);

private:
    QUrl image_;
    QList<QRectF> rects_;
    QSharedPointer<ImageData> imageData_;
    QSharedPointer<Resource> lifeToken_;
};

#endif // PAGEBOXPAGEITEM_H
