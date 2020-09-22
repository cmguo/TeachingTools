#include "pageanimcanvas.h"
#include "pageboxdocitem.h"

#include <QTimeLine>
#include <QGraphicsScene>

PageAnimCanvas::PageAnimCanvas(QGraphicsItem *parent)
    : QGraphicsRectItem(parent)
    , document_(static_cast<PageBoxDocItem*>(parent))
    , pageCanvas_(document_->childItems().first())
{
}

PageAnimCanvas::~PageAnimCanvas()
{
    if (timeLine_) {
        timeLine_->stop();
        delete timeLine_;
        timeLine_ = nullptr;
    }
}

void PageAnimCanvas::setDirection(PageAnimCanvas::AnimateDirection dir)
{
    direction_ = dir;
}

bool PageAnimCanvas::inAnimate() const
{
    return timeLine_;
}

void PageAnimCanvas::setAfterPageSwitch(bool after)
{
    afterPageSwitch_ = after;
    switchPage_ = after;
}

void PageAnimCanvas::startAnimate()
{
    QRectF vrect = document_->mapFromScene(scene()->sceneRect()).boundingRect();
    qreal width1 = childItems().size() == 1
            ? document_->pageSize().width()
            : document_->pageSize2().width();
    qreal width2 = document_->pageSize2().width();
    scale_ = vrect.width() / scene()->width();
    qreal diff1 = 200 + width2;
    qreal diff2 = 0;
    if (direction_ == LeftToRight) {
        diff2 = vrect.right() - (width1 + width2) / 2;
    } else {
        diff1 = -diff1;
        diff2 = vrect.left() - (width2 - width1) / 2;
    }
    if (vrect.width() > width1)
        diff2 = vrect.center().x() - width2 / 2.0;
    offset_ = diff1 + diff2;
    setX(-diff1);
    setOpacity(0);
    if (afterPageSwitch_)
        release();
}

bool PageAnimCanvas::move(QPointF const & offset)
{
    qreal o = pageCanvas_->x() + offset.x() * scale_;
    if (direction_ == LeftToRight ? o < 0 : o > 0)
        return false;
    qreal v = o / offset_;
    setX(x() + o - pageCanvas_->x());
    setOpacity(v);
    pageCanvas_->setX(o);
    pageCanvas_->setOpacity(1.0 - v);
    return true;
}

bool PageAnimCanvas::release()
{
    qreal x = pageCanvas_->x();
    if (!switchPage_)
        switchPage_ = qAbs(x) >= scene()->width() * scale_ / 4;
    timeLine_ = new QTimeLine;
    if (switchPage_) {
        timeLine_->setDuration(static_cast<int>(500.0 * (offset_ - x) / offset_));
        timeLine_->setFrameRange(static_cast<int>(x), static_cast<int>(offset_));
    } else {
        timeLine_->setDuration(static_cast<int>(500.0 * x / offset_));
        timeLine_->setFrameRange(static_cast<int>(x), 0);
    }
    QObject::connect(timeLine_, &QTimeLine::finished, [=]() {
        stopAnimate();
    });
    QObject::connect(timeLine_, &QTimeLine::frameChanged, [=] (int o) {
        qreal v = o / offset_;
        setX(this->x() + o - pageCanvas_->x());
        setOpacity(v);
        pageCanvas_->setX(o);
        pageCanvas_->setOpacity(1.0 - v);
    });
    timeLine_->start();
    return switchPage_;
}

void PageAnimCanvas::stopAnimate()
{
    if (switchPage_) {
        auto items1 = this->childItems();
        auto items2 = pageCanvas_->childItems();
        for (QGraphicsItem * i : items1)
            i->setParentItem(pageCanvas_);
        for (QGraphicsItem * i : items2)
            i->setParentItem(this);
    }
    bool finish = timeLine_ && timeLine_->state() == QTimeLine::NotRunning;
    if (timeLine_) {
        timeLine_->stop();
        delete timeLine_;
        timeLine_ = nullptr;
    }
    setX(-pageCanvas_->x());
    setOpacity(0);
    pageCanvas_->setX(0);
    pageCanvas_->setOpacity(1.0);
    document_->destroyAnimCanvas(this, finish);
}

