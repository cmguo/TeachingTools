#include "pageboxitem.h"
#include "pageboxdocitem.h"
#include "pageboxcontrol.h"
#include "pageboxtoolbar.h"

#include <views/toolbarwidget.h>
#include <core/control.h>
#include <core/resourceview.h>
#include <core/resourcetransform.h>

#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsProxyWidget>
#include <QPen>
#include <QDebug>

PageBoxItem::PageBoxItem(QGraphicsItem * parent)
    : QGraphicsRectItem(parent)
    , sizeMode_(FixedSize)
{
    setFlags(ItemClipsToShape | ItemClipsChildrenToShape);
    setPen(QPen(Qt::NoPen));
    //setBrush(QColor(240, 240, 240));
    //setBrush(QColor(128, 128, 128));
    setRect({-150, -300, 300, 600});

    document_ = new PageBoxDocItem(this);

    QGraphicsProxyWidget * proxy = new QGraphicsProxyWidget(this);
    toolBar_ = new PageBoxToolBar;
    toolBar_->setIsFull(true);
    proxy->setWidget(toolBar_);
    QPointF pos = -proxy->boundingRect().center();
    proxy->setTransform(QTransform::fromTranslate(pos.x(), pos.y()));
    toolBarProxy_ = proxy;

    QObject::connect(toolBar_->preBtn, SIGNAL(clicked()), this, SLOT(toolButtonClicked()));
    QObject::connect(toolBar_->nextBtn, SIGNAL(clicked()), this, SLOT(toolButtonClicked()));
    QObject::connect(toolBar_->exitBtn, SIGNAL(clicked()), this, SLOT(toolButtonClicked()));
    QObject::connect(toolBar_, SIGNAL(fullStateChanged(bool)), this, SLOT(scaleModelChanged(bool)));
    QObject::connect(document_, SIGNAL(currentPageChanged(int)), this, SLOT(documentPageChanged(int)));
    QObject::connect(document_, SIGNAL(sizeChanged(QSizeF)), this, SLOT(documentSizeChanged(QSizeF)));
}

bool PageBoxItem::selectTest(QPointF const & point)
{
    return !document_->contains(mapToItem(document_, point))
            && !toolBarProxy_->contains(mapToItem(toolBarProxy_, point));
}

void PageBoxItem::setSizeMode(PageBoxItem::SizeMode mode)
{
    sizeMode_ = mode;
}

void PageBoxItem::sizeChanged()
{
    document_->rescale();
}

QRectF PageBoxItem::visibleRect() const
{
    return sizeMode_ == LargeCanvas ? QRectF(rect().topLeft(), scene()->sceneRect().size())
                                    : boundingRect();
}

void PageBoxItem::toolButtonClicked()
{
    if (toolBar_->preBtn == sender()) {
         document_->previousPage();
    } else if(toolBar_->nextBtn){
         document_->nextPage();
    } else if(toolBar_->exitBtn){
         Control::fromItem(this)->resource()->removeFromPage();
    }
}

void PageBoxItem::scaleModelChanged(bool isfull)
{
   if(!isfull) {
      document_->setScaleMode(PageBoxDocItem::WholePage);
      //document_->setLayoutMode(PageBoxDocItem::Single);
   } else {
      document_->setScaleMode(PageBoxDocItem::FitLayout);
      //document_->setLayoutMode(PageBoxDocItem::Duplex);
   }
}

void PageBoxItem::documentPageChanged(int page)
{
    toolBar_->updateProgressText(QString::fromLocal8Bit("%1/%2").arg(page + 1).arg(document_->pageCount()));
}

void PageBoxItem::documentSizeChanged(const QSizeF &size)
{
    if (sizeMode_ == FixedSize) {
        return;
    }
    QSizeF size2 = calcSize(size);
    QRectF rect(QPointF(0, 0), size2);
    rect.moveCenter(QPointF(0, 0));
    setRect(rect);
    qDebug() << "PageBoxItem documentSizeChanged" << rect;
    PageBoxControl * control = qobject_cast<PageBoxControl*>(Control::fromItem(this));
    if (control) {
        control->sizeChanged();
    }
}

QSizeF PageBoxItem::calcSize(QSizeF const &size)
{
    QRectF rect = this->rect();
    if (sizeMode_ == LargeCanvas) {
        PageBoxControl * control = qobject_cast<PageBoxControl*>(Control::fromItem(this));
        if (control->flags() & Control::RestoreSession)
            return rect.size();
        QSizeF docSize = document_->documentSize();
        document_->rescale();
        document_->transferToManualScale();
        if (document_->direction() == PageBoxDocItem::Horizontal) {
            qreal s = size.width() / rect.width();
            return QSizeF(docSize.width() / s, rect.height());
        } else {
            qreal s = size.height() / rect.height();
            return QSizeF(rect.width(), docSize.height() / s);
        }
    } else {
        qreal s = document_->direction() == PageBoxDocItem::Horizontal
                ? rect.height() / size.height() : rect.width() / size.width();
        return size * s;
    }
}

void PageBoxItem::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    if (!document_->contains(mapToItem(document_, event->pos()))) {
        QGraphicsRectItem::mousePressEvent(event);
        return;
    }
    start_ = event->pos();
    type_ = 1;
}

void PageBoxItem::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    if (type_ == 0) {
        QGraphicsRectItem::mouseMoveEvent(event);
        return;
    }
    QPointF pt = event->pos();
    QPointF d = pt - start_;
    QRectF vrect = rect();
    QRectF rect = mapFromItem(document_, document_->boundingRect()).boundingRect();
    switch (type_) {
    case 1:
    case 2:
        if (rect.left() + d.x() > vrect.left())
            d.setX(rect.left() > vrect.left() ? 0 : vrect.left() - rect.left());
        else if (rect.right() + d.x() < vrect.right())
            d.setX(rect.right() < vrect.right() ? 0 : vrect.right() - rect.right());
        if (rect.top() + d.y() > vrect.top())
            d.setY(rect.top() > vrect.top() ? 0 : vrect.top() - rect.top());
        else if (rect.bottom() + d.y() < vrect.bottom())
            d.setY(rect.bottom() < vrect.bottom() ? 0 : vrect.bottom() - rect.bottom());
        document_->transform_->translate(d);
        type_ = 2;
        break;
    }
    start_ = pt;
}

void PageBoxItem::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    if (type_ == 0) {
        QGraphicsRectItem::mouseReleaseEvent(event);
        return;
    }
    switch (type_) {
    case 1:
        document_->hit(mapToItem(document_, start_));
        break;
    }
    type_ = 0;
}

void PageBoxItem::wheelEvent(QGraphicsSceneWheelEvent *event)
{
    if (!document_->contains(mapToItem(document_, event->pos()))) {
        QGraphicsRectItem::wheelEvent(event);
        return;
    }
    QRectF vrect = rect();
    QRectF rect = mapFromItem(document_, document_->boundingRect()).boundingRect();
    qreal d = event->delta();
    if (document_->scaleMode() == PageBoxDocItem::WholePage) {
        if (d > 0)
            document_->previousPage();
        else
            document_->nextPage();
        return;
    }
    if (document_->direction() == PageBoxDocItem::Vertical) {
        if (rect.top() + d > vrect.top())
            d = rect.top() > vrect.top() ? 0 : vrect.top() - rect.top();
        else if (rect.bottom() + d < vrect.bottom())
            d = rect.bottom() < vrect.bottom() ? 0 : vrect.bottom() - rect.bottom();
        document_->moveBy(0, d);
    } else {
        if (rect.left() + d > vrect.left())
            d = rect.left() > vrect.left() ? 0 : vrect.left() - rect.left();
        else if (rect.right() + d < vrect.right())
            d = rect.right() < vrect.right() ? 0 : vrect.right() - rect.right();
        document_->moveBy(d, 0);
    }
}
