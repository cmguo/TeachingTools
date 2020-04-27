#ifndef PAGEBOXDOCITEM_H
#define PAGEBOXDOCITEM_H

#include "TeachingTools_global.h"

#include <core/toolbuttonprovider.h>

#include <QGraphicsRectItem>

class QAbstractItemModel;
class QPropertyBindings;
class PageBoxPlugin;
class PageBoxPageItem;
class PageNumberWidget;

class TEACHINGTOOLS_EXPORT PageBoxDocItem : public ToolButtonProvider, public QGraphicsRectItem
{
    Q_OBJECT
public:
    enum LayoutMode
    {
        Single,
        Continuous,
        Duplex, // except first page
        DuplexSingle,
    };

    enum Direction
    {
        Vertical,
        Horizontal,
    };

public:
    PageBoxDocItem(QGraphicsItem * parent = nullptr);

    virtual ~PageBoxDocItem() override;

public:
    void setPageSize(QSizeF size);

    void setDirection(Direction direction);

    void setLayoutMode(LayoutMode mode);

    void setPadding(qreal pad);

public:
    QSizeF pageSize() const { return pageSize_; }

    QSizeF pageSize2() const { return pageSize2_; }

    QSizeF documentSize() const;

    Direction direction() const { return direction_; }

    LayoutMode layoutMode() const { return layoutMode_; }

    qreal padding() const { return padding_; }

    int pageCount() const;

    int curPage() const { return curPage_; }

public:
    void addPlugin(PageBoxPlugin* plugin);

    void removePlugin(PageBoxPlugin* plugin);

    QList<PageBoxPlugin *> plugins() const { return plugins_; }

    PageNumberWidget * pageNumberWidget() const { return pageNumber_; }

public:
    qreal requestScale(QSizeF const & borderSize, bool whole);

    void visiblePositionHint(QGraphicsItem * from, QPointF const & pos);

public:
    void setItems(QAbstractItemModel * model);

    void setItemBindings(QPropertyBindings * bindings);

    void reset();

    void resetCurrent();

public slots:
    void nextPage();

    void previousPage();

    void frontPage();

    void backPage();

    void goToPage(int page);

public:
    bool hit(QPointF const & point);

public:
    QByteArray saveState();

    void restoreState(QByteArray data);

signals:
    void layoutModeChanged();

    void pageCountChanged(int count);

    void currentPageChanged(int page);

    void pageSize2Changed(QSizeF const & size);

    // request visible position in document, if x < 0, x is not changed, y same
    void requestPosition(QPointF const & pos);

protected:
    virtual void getToolButtons(QList<ToolButton *> &buttons, const QList<ToolButton *> &parents = {}) override;

    virtual bool handleToolButton(QList<ToolButton *> const & buttons);

    virtual void getToolButtons(QList<ToolButton *> &buttons, ToolButton *parent) override;

protected:
    friend class PageBoxItem;

    void clear();

    void relayout();

protected:
    virtual void onPageSize2Changed(QSizeF const & size);

    virtual void onVisibleCenterChanged(QPointF const & pos);

    virtual void onCurrentPageChanged(int last, int cur);

private slots:
    void resourceInserted(QModelIndex const &parent, int first, int last);

    void resourceRemoved(QModelIndex const &parent, int first, int last);

    void resourceMoved(QModelIndex const &parent, int start, int end,
                       QModelIndex const &destination, int row);

private:
    void setDefaultImage(PageBoxPageItem* pageItem1, PageBoxPageItem* pageItem2 = nullptr);

private:
    QAbstractItemModel * model_;

    QSizeF pageSize_;
    QSizeF pageSize2_;
    Direction direction_;
    LayoutMode layoutMode_;
    qreal padding_; // Continuous mode
    int curPage_;

private:
    QGraphicsRectItem * pageCanvas_;
    PageNumberWidget * pageNumber_;
    QList<PageBoxPlugin *> plugins_;

private:
    QPropertyBindings * itemBindings_;
};

#endif // PAGEBOXDOCITEM_H
