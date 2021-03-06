#include "inkstrokecontrol.h"
#include "inkstrokefilter.h"

#include <widget/qsshelper.h>
#include <widget/toolbarwidget.h>

#include <Windows/Controls/inkcanvas.h>

#include <QGraphicsProxyWidget>
#include <QGraphicsSceneMouseEvent>
#include <QPushButton>
#include <QGraphicsScene>
#include <QElapsedTimer>
#include <QTimer>
#include <QApplication>

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
            checkTip(me.scenePos(), event->type() == QEvent::GraphicsSceneMouseDoubleClick);
        }
        return false;
    }

    if(event->type() == QEvent::GraphicsSceneMousePress){
        setData(1000, true);
    }else if(event->type() == QEvent::GraphicsSceneMouseMove){
        setData(1000, false);
    }else if( event->type() == QEvent::GraphicsSceneMouseRelease && data(1000).toBool()) {

        QGraphicsSceneMouseEvent & me = static_cast<QGraphicsSceneMouseEvent&>(*event);
        Qt::MouseEventSource source = me.source();
        me.setSource(Qt::MouseEventNotSynthesized);
        QPointF pos = me.pos();
        QPointF lastPos = me.lastPos();
        QWeakPointer<int> l = life_; // may switch page and destroyed

        QList<QGraphicsItem*> items = scene()->items(me.scenePos());
        items = items.mid(items.indexOf(watched) + 1);
        for (QGraphicsItem * item : items) {
            if (item == rootItem_)
                break;
            // avoid leave point in another ink canvas
            if (InkCanvas::fromItem(item))
                continue;
            if(item->data(100000001).toInt() == 1) //support filter press & release
                continue;

            me.setPos(item->mapFromScene(me.scenePos()));
            me.setLastPos(item->mapFromScene(me.lastScenePos()));
            me.accept();

            QGraphicsSceneMouseEvent m(QEvent::GraphicsSceneMousePress);
            m.setPos(me.pos());
            m.setSource(me.source());
            m.setModifiers(me.modifiers());
            m.setFlags(me.flags());
            m.setButtons(me.buttons());
            m.setButton(me.button());
            m.setLastPos(me.lastPos());
            m.setScenePos(me.scenePos());
            m.setScreenPos(me.screenPos());
            m.accept();
            sending_ = true;
            scene()->sendEvent(item, &m);
            sending_ = false;

            if (!m.isAccepted()) {
                continue;
            }

            sending_ = true;
            scene()->sendEvent(item, event);
            sending_ = false;
            break;
        } // for
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
//        layout->addWidget(new QLabel("??????????????????????"));
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
    void check(QPointF const & pos, bool dblClick, QGraphicsItem * inkCanvas)
    {
        if (!dblClick && (!timer_.isValid() || timer_.elapsed() > 1000)) {
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

void InkStrokeFilter::checkTip(const QPointF &pos, bool dblClick)
{
    if (tipItem == nullptr) {
        tipItem = new TipItem();
    }
    if (tipItem->scene() != scene()) {
        scene()->addItem(tipItem);
        tipItem->hide();
    }
    tipItem->check(pos, dblClick, parentItem());
}

void InkStrokeFilter::paint(QPainter *, const QStyleOptionGraphicsItem *, QWidget *)
{
}
