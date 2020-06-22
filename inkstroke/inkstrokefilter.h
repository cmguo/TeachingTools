#ifndef INKSTROKEFILTER_H
#define INKSTROKEFILTER_H

#include <QGraphicsItem>

class InkStrokeFilter : public QGraphicsItem
{
public:
    InkStrokeFilter(QGraphicsItem * parentItem);

    virtual ~InkStrokeFilter() override;

public:
    bool sendingEvent() const { return sending_; }

public:
    QRectF boundingRect() const override { return QRectF(); }

    void paint(QPainter *, const QStyleOptionGraphicsItem *, QWidget *) override;

    bool sceneEventFilter(QGraphicsItem *watched, QEvent *event) override;

    QVariant itemChange(GraphicsItemChange change, const QVariant &value) override;

private:
    void checkTip(QPointF const & pos);

private:
    QSharedPointer<int> life_;
    QGraphicsItem * rootItem_ = nullptr;
    bool sending_ = false;
};

#endif // INKSTROKEFILTER_H
