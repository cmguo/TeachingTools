#include "loupeitem.h"

#include <QEvent>
#include <QPainter>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QPaintEvent>
#include <QDebug>

static constexpr int SIZE = 140;

LoupeItem::LoupeItem(QGraphicsItem *parent)
    : QGraphicsItem(parent)
    , scale_(2)
{
    constexpr QSize itemSize(20, 20);

    setFlag(ItemHasNoContents);
    setFlag(ItemSendsGeometryChanges);
    content_ = QPixmap(SIZE, SIZE);
    content_.fill(Qt::red);

    image_ = new QGraphicsPixmapItem(this);
    image_->setPixmap(QPixmap(":/teachingtools/image/loupe.png"));
    image_->setOffset({-85, -85});

    glass_ = new QGraphicsPixmapItem(this);
    glass_->setPixmap(content_);
    glass_->setOffset({-SIZE / 2, -SIZE / 2});

    incScale_ = new QGraphicsPixmapItem(this);
    incScale_->setPixmap(QPixmap(":/teachingtools/icon/icon_plus.png").scaled(itemSize, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    incScale_->setAcceptedMouseButtons(Qt::LeftButton);
    incScale_->setPos(75, 75);

    decScale_ = new QGraphicsPixmapItem(this);
    decScale_->setPixmap(QPixmap(":/teachingtools/icon/icon_minus.png").scaled(itemSize, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    decScale_->setAcceptedMouseButtons(Qt::LeftButton);
    decScale_->setPos(105, 105);
}

void LoupeItem::setScale(qreal scale)
{
    scale_ = scale;
    update(contentRect_);
}

QRectF LoupeItem::boundingRect() const
{
    return image_->boundingRect();
}

QVariant LoupeItem::itemChange(QGraphicsItem::GraphicsItemChange change, const QVariant &value)
{
    switch (change) {
    case ItemVisibleHasChanged:
        incScale_->installSceneEventFilter(this);
        decScale_->installSceneEventFilter(this);
        scene()->views().front()->installEventFilter(this);
        break;
    case ItemPositionHasChanged:
    case ItemTransformHasChanged:
        qDebug() << "LoupeItem::itemChange ItemPositionHasChanged";
        contentRect_ = mapToScene(glass_->boundingRect()).boundingRect();
        update(contentRect_);
        break;
    default:
        break;
    }
    return value;
}

bool LoupeItem::sceneEventFilter(QGraphicsItem *watched, QEvent *event)
{
    if (event->type() == QEvent::GraphicsSceneMousePress) {
        return true;
    } else if (event->type() == QEvent::GraphicsSceneMouseRelease) {
        if (watched == incScale_) {
            setScale(scale_ + 0.5);
        } else if (scale_ > 1.2) {
            setScale(scale_ - 1.5);
        }
        return true;
    }
    return false;
}

bool LoupeItem::eventFilter(QObject *watched, QEvent *event)
{
    (void) watched;
    if (event->type() == QEvent::Paint) {
        QPaintEvent & e = static_cast<QPaintEvent &>(*event);
        qDebug() << "LoupeItem::eventFilter Paint" << e.rect();
        QRectF rect = static_cast<QGraphicsView*>(watched)->mapToScene(e.rect()).boundingRect();
        if (rect.intersects(contentRect_)) {
            update(rect & contentRect_);
        }
    }
    return false;
}

void LoupeItem::paint(QPainter *, const QStyleOptionGraphicsItem *, QWidget *)
{
}

void LoupeItem::transformChanged()
{
    itemChange(ItemTransformHasChanged, QVariant());
}

void LoupeItem::update(const QRectF &rect)
{
    qDebug() << "LoupeItem::update";
    QPainter painter(&content_);
    setVisible(false);
    scene()->render(&painter, {0, 0, SIZE, SIZE}, contentRect_);
    setVisible(true);
//    glass_->update();
}
