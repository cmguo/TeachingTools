#include "hanzicontrol.h"
#include "hanziitem.h"
#include "core/resourceview.h"
#include "core/optiontoolbuttons.h"

#include <QGraphicsPathItem>
#include <QPainter>
#include <QPen>
#include <QUrl>
#include <QDebug>

static constexpr char const * toolstr =
        "next()|下一笔|;"
        "anim()|动画|;"
        "radical()|部首|;"
        "pinyin()|拼音|;"
        ;

HanziControl::HanziControl(ResourceView *res)
    : Control(res, {KeepAspectRatio, FixedOnCanvas})
{
    setToolsString(toolstr);
}

ControlView *HanziControl::create(ControlView *)
{
    HanziItem * item = new HanziItem(res_->url().path().front());
    return item;
}

void HanziControl::next()
{
    HanziItem * item = static_cast<HanziItem*>(item_);
    item->next();
    if (timer_)
        killTimer(timer_);
}

void HanziControl::anim()
{
    HanziItem * item = static_cast<HanziItem*>(item_);
    item->clear();
    if (timer_)
        killTimer(timer_);
    timer_ = startTimer(100);
}

void HanziControl::radical()
{
    HanziItem * item = static_cast<HanziItem*>(item_);
    item->radical();
    if (timer_)
        killTimer(timer_);
}

void HanziControl::pinyin()
{
    HanziItem * item = static_cast<HanziItem*>(item_);
    item->pinyin();
}

void HanziControl::timerEvent(QTimerEvent *event)
{
    if (event->timerId() == timer_) {
        HanziItem * item = static_cast<HanziItem*>(item_);
        if (item->anim()) {
            killTimer(event->timerId());
            timer_ = 0;
        }
    }
}

