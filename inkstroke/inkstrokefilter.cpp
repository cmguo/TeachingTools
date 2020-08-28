#include "inkstrokecontrol.h"
#include "inkstrokefilter.h"

#include <views/qsshelper.h>
#include <views/toolbarwidget.h>

#include <Windows/Controls/inkcanvas.h>

#include <QGraphicsProxyWidget>
#include <QGraphicsSceneMouseEvent>
#include <QPushButton>
#include <QGraphicsScene>
#include <QElapsedTimer>
#include <QTimer>

InkStrokeFilter::InkStrokeFilter(QGraphicsItem *parentItem)
    : QGraphicsItem(parentItem)
    , life_(new int)
{
    setFlag(ItemHasNoContents, true);
    hide();
}

bool InkStrokeFilter::sceneEventFilter(QGraphicsItem *watched, QEvent *event)
{
    if (watched != parentItem()) {
        // filtering WhiteCanvas
        if ((event->type() == QEvent::GraphicsSceneMousePress
                && !event->isAccepted())
                || event->type() == QEvent::GraphicsSceneMouseDoubleClick) {
            QGraphicsSceneMouseEvent & me = static_cast<QGraphicsSceneMouseEvent&>(*event);
            checkTip(me.scenePos());
        }
        return false;
    }
    if (event->type() == QEvent::GraphicsSceneMousePress
            || event->type() == QEvent::GraphicsSceneMouseRelease) {
        QGraphicsSceneMouseEvent & me = static_cast<QGraphicsSceneMouseEvent&>(*event);
        QList<QGraphicsItem*> items = scene()->items(me.scenePos());
        items = items.mid(items.indexOf(watched) + 1);
        Qt::MouseEventSource source = me.source();
        me.setSource(Qt::MouseEventNotSynthesized);
        QPointF pos = me.pos();
        QPointF lastPos = me.lastPos();
        QWeakPointer<int> l = life_; // may switch page and destroyed
        if (event->type() == QEvent::GraphicsSceneMousePress) {
            for (QGraphicsItem * item : items) {
                if (item == rootItem_)
                    break;
                // avoid leave point in another ink canvas
                if (InkCanvas::fromItem(item))
                    continue;
                if(item->data(100000001).toInt()==1)
                    continue;
                me.setPos(item->mapFromScene(me.scenePos()));
                me.setLastPos(item->mapFromScene(me.lastScenePos()));
                me.accept();
                sending_ = true;
                scene()->sendEvent(item, event);
                sending_ = false;
                if (event->isAccepted()) {
                    setData(1000, QVariant::fromValue(item));
                    break;
                }
            }
        } else {
            QGraphicsItem * item = data(1000).value<QGraphicsItem *>();
            setData(1000, QVariant());
            if (items.contains(item)) {
                me.setPos(item->mapFromScene(me.scenePos()));
                me.setLastPos(item->mapFromScene(me.lastScenePos()));
                me.accept();
                sending_ = true;
                if(item->data(100000001).toInt()!=1)
                scene()->sendEvent(item, event);
                sending_ = false;
            }
        }
        if (l.isNull())
            return true;
        me.setSource(source);
        me.setPos(pos);
        me.setLastPos(lastPos);
        event->ignore();
    }
    return false;
}

QVariant InkStrokeFilter::itemChange(QGraphicsItem::GraphicsItemChange change, const QVariant &value)
{
    if (change == ItemSceneHasChanged) {
        if (rootItem_ == nullptr) {
            rootItem_ = parentItem();
            while (rootItem_->parentItem()) {
                rootItem_ = rootItem_->parentItem();
            }
        }
    }
    return value;
}

class TipItem : public QGraphicsProxyWidget
{
public:
    TipItem()
    {
        QFrameEx* widget = new QFrameEx;
        widget->setObjectName("inkstroketip");
        widget->setWindowFlag(Qt::FramelessWindowHint);
        widget->setStyleSheet(QssHelper(":/teachingtools/qss/inkstroketip.qss"));
//        QLayout* layout = new QVBoxLayout(widget);
//        widget->setLayout(layout);
//        layout->setSpacing(20);
//        layout->addWidget(new QLabel("是否切换到画笔?"));
        QPushButton * button = new QPushButton(widget);
        button->setAttribute(Qt::WA_AcceptTouchEvents);
        QObject::connect(button, &QPushButton::clicked, [this]() {
            qobject_cast<InkStrokeControl*>(Control::fromItem(inkCanvas_))
                    ->setEditingMode(InkCanvasEditingMode::Ink);
            hide();
        });
//        layout->addWidget(button);
        setWidget(widget);
    }
    void check(QPointF const & pos, QGraphicsItem * inkCanvas)
    {
        if (!timer_.isValid() || timer_.elapsed() > 1000) {
            timer_.restart();
            return;
        }
        timer_.invalidate();
        int tipSeq = ++tipSeq_;
        inkCanvas_ = inkCanvas;
        setPos(pos);
        show();
        setFocus();
        QTimer::singleShot(3000, widget(), [this, tipSeq]() {
            if (tipSeq == tipSeq_)
                hide();
        });
    }
    void cancel()
    {
        hide();
        inkCanvas_ = nullptr;
    }
protected:
    virtual bool sceneEvent(QEvent * event)
    {
        if (event->type() == QEvent::FocusOut)
            hide();
        return QGraphicsProxyWidget::sceneEvent(event);
    }
private:
    QElapsedTimer timer_;
    int tipSeq_ = 0;
    QGraphicsItem * inkCanvas_ = nullptr;
};

static TipItem * tipItem = nullptr;

InkStrokeFilter::~InkStrokeFilter()
{
    if (tipItem)
        tipItem->cancel();
}

void InkStrokeFilter::checkTip(const QPointF &pos)
{
    if (tipItem == nullptr) {
        tipItem = new TipItem();
    }
    if (tipItem->scene() != scene()) {
        scene()->addItem(tipItem);
        tipItem->hide();
    }
    tipItem->check(pos, parentItem());
}

void InkStrokeFilter::paint(QPainter *, const QStyleOptionGraphicsItem *, QWidget *)
{
}
