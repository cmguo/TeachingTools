#ifndef PAGEBOXPAGEITEM_H
#define PAGEBOXPAGEITEM_H

#include <QGraphicsPixmapItem>
#include <QUrl>
#include <QList>

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

private:
    QUrl image_;
    QList<QRectF> rects_;
    QSharedPointer<int> lifeToken_;
};

#endif // PAGEBOXPAGEITEM_H
