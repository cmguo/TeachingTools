#ifndef HANZIITEM_H
#define HANZIITEM_H

#include "hanzi.h"

#include <QGraphicsRectItem>
#include <QMediaPlayer>
#include <QMediaPlaylist>

class HanziItem : public QGraphicsRectItem
{
public:
    HanziItem(QChar character, QGraphicsItem * parent = nullptr);

public:
    void next();

    void clear();

    bool anim();

    void radical();

    void pinyin();

protected:
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = nullptr) override;

    enum State {
        Highlight = Qt::red,
        Normal = Qt::white,
        Median,
        Dark = Qt::darkGray,
        Hide = Qt::transparent,
        LastState
    };

private:
    static QPainterPath subPath(QPainterPath const & ph, qreal length);

private:
    struct Stroke {
        QPainterPath path;
        QPainterPath medians;
        int length;
        State state = Highlight;
    };

    Hanzi hanzi_;
    int step_ = -1;
    int step2_ = 0;
    QVector<Stroke> strokes_;
    QMediaPlayer player_;
    QMediaPlaylist list_;
};

#endif // HANZIITEM_H
