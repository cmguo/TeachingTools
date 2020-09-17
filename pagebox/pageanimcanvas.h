#ifndef PAGEANIMCANVAS_H
#define PAGEANIMCANVAS_H

#include <QGraphicsRectItem>

class PageBoxDocItem;
class QTimeLine;

class PageAnimCanvas : public QGraphicsRectItem
{
public:
    enum AnimateDirection
    {
        LeftToRight,
        RightToLeft,
    };

    explicit PageAnimCanvas(QGraphicsItem *parent = nullptr);

    virtual ~PageAnimCanvas() override;

public:
    AnimateDirection direction() const { return direction_; }

    void setDirection(AnimateDirection dir);

    bool afterPageSwitch() const { return afterPageSwitch_; }

    bool inAnimate() const;

    //  true: if canvas is holding new page
    //  false: if canvas is holding old page, switch to new page after animation
    void setAfterPageSwitch(bool after = true);

    bool switchPage() const { return switchPage_; }

    void startAnimate();

    // return true if reverted
    bool move(QPointF const & offset);

    bool release();

    void stopAnimate();

private:
    PageBoxDocItem * document_;
    QGraphicsItem * pageCanvas_;
    AnimateDirection direction_;
    bool afterPageSwitch_ = false;
    bool switchPage_ = false;
    qreal offset_ = 0;
    qreal scale_ = 1.0;
    QTimeLine * timeLine_ = nullptr;
};

#endif // PAGEANIMCANVAS_H
