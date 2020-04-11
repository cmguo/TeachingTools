#ifndef PAGEBOXITEM_H
#define PAGEBOXITEM_H

#include <core/toolbuttonprovider.h>

#include <QGraphicsRectItem>

class PageBoxDocItem;
class PageBoxToolBar;
class PageNumberWidget;
class PageBoxPlugin;

class PageBoxItem : public ToolButtonProvider, public QGraphicsRectItem
{
    Q_OBJECT

    Q_PROPERTY(int pageNumber READ pageNumber  WRITE setPageNumber)
    Q_PROPERTY(QByteArray pageBoxState READ pageBoxState  WRITE setPageBoxState)

public:
    PageBoxItem(QGraphicsItem * parent = nullptr);

    virtual ~PageBoxItem() override;

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

    void setPlugin(PageBoxPlugin* plugin);

public slots:
    void duplex();

    void single();

    void scaleUp();

    void scaleDown();

    void exit();

protected:
    int pageNumber();

    void setPageNumber(int n);

    QByteArray pageBoxState();

    void setPageBoxState(QByteArray state);

public:
    virtual void getToolButtons(QList<ToolButton *> &buttons, ToolButton *parent) override;

    virtual void updateToolButton(ToolButton *button) override;

private:
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
    PageNumberWidget * pageNumber_;

private:
    SizeMode sizeMode_;
    QPointF start_;
    QRectF direction_;
    int type_;
};

#endif // PAGEBOXITEM_H
