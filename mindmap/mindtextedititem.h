#ifndef MINDTEXTEDITITEM_H
#define MINDTEXTEDITITEM_H

#include <QGraphicsTextItem>

class MindNodeView;

class MindTextEditItem : public QGraphicsTextItem
{
    Q_OBJECT
public:
    MindTextEditItem(QGraphicsItem * parent);

public:
    void attachTo(MindNodeView * nodeView, QPointF const & pos = {});

signals:
    void editEnded();

private:
    void focusOutEvent(QFocusEvent *event) override;

    void keyPressEvent(QKeyEvent *event) override;

    void keyReleaseEvent(QKeyEvent *event) override;

private:
    void finish(bool cancel = false);

private:
    MindNodeView *nodeView_ = nullptr;
};

#endif // MINDTEXTEDITITEM_H
