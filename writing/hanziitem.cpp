#include "hanziitem.h"

#include <QPainter>
#include <QLine>
#include <QMediaPlaylist>
#include <QFile>

HanziItem::HanziItem(QChar character, QGraphicsItem * parent)
    : QGraphicsRectItem(parent)
    , hanzi_(character)
{
    setRect(hanzi_.VIEW_BOX);
    setPen(Qt::NoPen);
    QVector<QPainterPath> strokes = hanzi_.strokes();
    int i = 0;
    for (QPainterPath & ph : strokes) {
        auto md = hanzi_.medians(i);
        int length = static_cast<int>(floor(md.length()));
        strokes_.append(Stroke{ph, md, length});
        ++i;
    }
    player_.setPlaylist(&list_);
}

void HanziItem::next()
{
    if (step_ < 0 || step_ >= strokes_.size()) {
        clear();
        return;
    }
    //update(strokes_[step].boundingRect());
    strokes_[step_++].state = Normal;
    update();
}

void HanziItem::clear()
{
    for (auto & s : strokes_) s.state = Dark;
    step_ = 0;
    step2_ = 0;
    update();
}

bool HanziItem::anim()
{
    if (step_ < 0)
        return true;
    if (step2_ == 0) {
        strokes_[step_].state = Median;
        step2_ += 30;
    } else if (step2_ >= strokes_[step_].length) {
        if (step_ < strokes_.size()) {
            strokes_[step_++].state = Normal;
            step2_ = 0;
        }
        update();
        return step_ >= strokes_.size();
	} else {
        step2_ += 30;
    }
    update();
    return false;
}

void HanziItem::radical()
{
    step_ = -1;
    for (auto & s : strokes_) s.state = Dark;
    for (auto s : hanzi_.radical()) {
        strokes_[s].state = Highlight;
    }
    update();
}

void HanziItem::pinyin()
{
    list_.clear();
    for (QByteArray py: hanzi_.pinyin()) {
        QString file = "PinYinSound/" + py + ".mp3";
        if (QFile(file).exists())
            list_.addMedia(QUrl::fromLocalFile(file));
        else
            qDebug() << "HanziItem::pinyin" << file << "not exists!";
    }
    player_.play();
}

void HanziItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    QGraphicsRectItem::paint(painter, option, widget);
    QTransform old = painter->transform();
    painter->setTransform(hanzi_.TRANSFORM, true);
    State states[] = { Hide, Dark, Median, Normal, Highlight };
    for (State s : states) {
        painter->setBrush(Qt::GlobalColor(s == Median ? Dark : s));
        for (auto & st : strokes_) {
            if (st.state == s)
                painter->drawPath(st.path);
        }
    }
    if (step_ >= 0 && step_ < strokes_.size()) {
        Stroke & m = strokes_[step_];
        if (m.state == Median) {
            QPainterPath median = subPath(m.medians, step2_);
            QPainterPathStroker ps(QPen(QBrush(), 128, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
            median = ps.createStroke(median);
            median = median & m.path;
            painter->setBrush(Qt::GlobalColor(Highlight));
            painter->drawPath(median);
        }
    }
    painter->setTransform(old);
}

QPainterPath HanziItem::subPath(const QPainterPath &ph, qreal length)
{
    QPointF last = ph.elementAt(0);
    QPainterPath result;
    result.moveTo(last);
    for (int i = 1; i < ph.elementCount(); ++i) {
        QPointF cur = ph.elementAt(i);
        qreal l = QLineF(last, cur).length();
        if (length <= l) {
            cur = last + (cur - last) / l * length;
            result.lineTo(cur);
            break;
        }
        result.lineTo(cur);
        last = cur;
        length -= l;
    }
    return result;
}
