#ifndef MINDSPACING_H
#define MINDSPACING_H

#include "mindbaseview.h"

class MindNodeView;

class MindSpacing : public MindBaseView
{
public:
    QRectF boundingRect() const override;

    void draw(QPainter *painter, const QRectF &exposedRect) override;

public:
    MindNodeView * prev() { return prev_; }

    void setIgnore(MindNodeView * ignore);

    bool setPrevNext(MindNodeView * prev, MindNodeView * next);

    void saveLast(QRectF const & bounding);

private:
    MindNodeView * prev_ = nullptr;
    MindNodeView * next_ = nullptr;
    QRectF bounding_;
};

#endif // MINDSPACING_H
