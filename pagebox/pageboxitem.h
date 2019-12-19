#ifndef PAGEBOXITEM_H
#define PAGEBOXITEM_H

#include <QGraphicsRectItem>

class PageBoxDocItem;
class PageBoxToolBar;

class PageBoxItem : public QObject, public QGraphicsRectItem
{
    Q_OBJECT
public:
    PageBoxItem(QGraphicsItem * parent = nullptr);

public:
    PageBoxDocItem * document()
    {
        return document_;
    }

    QGraphicsItem * toolBar()
    {
        return toolBarProxy_;
    }

    bool selectTest(QPointF const & point);

    enum SizeMode
    {
        FixedSize,
        MatchContent,
        LargeCanvas
    };

    void setSizeMode(SizeMode mode);

    void sizeChanged();

    QRectF visibleRect() const;

private slots:
    void toolButtonClicked();

    void scaleModelChanged(bool isfull);

    void documentPageChanged(int page);

    void documentSizeChanged(QSizeF const & size);

private:
    virtual void mousePressEvent(QGraphicsSceneMouseEvent *event) override;

    virtual void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override;

    virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override;

    virtual void wheelEvent(QGraphicsSceneWheelEvent *event) override;

private:
    QSizeF calcSize(QSizeF const & size);

private:
    PageBoxDocItem * document_;
    PageBoxToolBar * toolBar_;
    QGraphicsItem * toolBarProxy_;
    SizeMode sizeMode_;
    QPointF start_;
    QRectF direction_;
    int type_;
};

#endif // PAGEBOXITEM_H
