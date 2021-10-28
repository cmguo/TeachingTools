#ifndef LOUPEITEM_H
#define LOUPEITEM_H

#include <QGraphicsObject>

class LoupeItem : public QObject, public QGraphicsItem
{
    Q_OBJECT

    Q_PROPERTY(qreal scale READ scale WRITE setScale)

public:
    LoupeItem(QGraphicsItem * parent = nullptr);

public:
    qreal scale() const { return scale_; }

    void setScale(qreal scale);

protected:
    QRectF boundingRect() const override;

    QVariant itemChange(GraphicsItemChange change, const QVariant &value) override;

    bool sceneEventFilter(QGraphicsItem *watched, QEvent *event) override;

    bool eventFilter(QObject *watched, QEvent *event) override;

    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = nullptr) override;

private:
    friend class LoupeTool;

    void transformChanged();

    void update(QRectF const & rect);

protected:
    qreal scale_;
    QRectF contentRect_;
    QPixmap content_;

private:
    QGraphicsPixmapItem * image_;
    QGraphicsPixmapItem * glass_;
    QGraphicsPixmapItem * incScale_;
    QGraphicsPixmapItem * decScale_;
};

#endif // LOUPEITEM_H
