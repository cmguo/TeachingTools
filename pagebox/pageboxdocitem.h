#ifndef PAGEBOXDOCITEM_H
#define PAGEBOXDOCITEM_H

#include "TeachingTools_global.h"

#include <QGraphicsRectItem>

class QAbstractItemModel;
class QPropertyBindings;
class PageBoxPlugin;
class ResourceTransform;
class PageBoxPageItem;

class TEACHINGTOOLS_EXPORT PageBoxDocItem : public QObject, public QGraphicsRectItem
{
    Q_OBJECT
public:
    enum LayoutMode
    {
        Single,
        Continuous,
        Duplex, // except first page
    };

    enum Direction
    {
        Vertical,
        Horizontal,
    };

    enum ScaleMode
    {
        WholePage,
        FitLayout, // when horizontal, fit heigth, else fit width
        ManualScale,
    };

public:
    PageBoxDocItem(QGraphicsItem * parent = nullptr);

public:
    void setPageSize(QSizeF size);

    void setDirection(Direction direction);

    void setLayoutMode(LayoutMode mode);

    void setScaleMode(ScaleMode mode);

    void setManualScale(qreal scale, bool changeMode = true);

    void transferToManualScale();

    void stepScale(bool up);

    bool canStepScale(bool up);

    void stepMiddleScale();

    void setPadding(qreal pad);

    void setPlugin(PageBoxPlugin* plugin);

public:
    QSizeF pageSize() const
    {
        return pageSize_;
    }

    QSizeF pageSize2() const
    {
        return pageSize2_;
    }

    QSizeF documentSize() const;

    Direction direction() const
    {
        return direction_;
    }

    LayoutMode layoutMode() const
    {
        return layoutMode_;
    }

    ScaleMode scaleMode() const
    {
        return scaleMode_;
    }

    qreal padding() const
    {
        return padding_;
    }

    qreal scale() const;

    QPointF offset() const;

    int pageCount() const;

    int curPage() const
    {
        return curPage_;
    }

    PageBoxPlugin * plugin() const
    {
        return plugin_;
    }

public:
    void setItems(QAbstractItemModel * model);

    void setItemBindings(QPropertyBindings * bindings);

    void reset();

public:
    void moveBy(qreal dx, qreal dy);

public slots:
    void nextPage();

    void previousPage();

    void frontPage();

    void backPage();

    void goToPage(int page);

public:
    bool hit(QPointF const & point);

public:
    ResourceTransform * detachTransform();

    QByteArray saveState();

    void restoreState(QByteArray data);

    void restorePosition();

signals:
    void layoutModeChanged();

    void pageCountChanged(int count);

    void currentPageChanged(int page);

    void sizeChanged(QSizeF const & size);

protected:
    friend class PageBoxItem;

    void clear();

    void relayout();

    void rescale();

private slots:
    void resourceInserted(QModelIndex const &parent, int first, int last);

    void resourceRemoved(QModelIndex const &parent, int first, int last);

    void resourceMoved(QModelIndex const &parent, int start, int end,
                       QModelIndex const &destination, int row);

private:
    void setDefaultImage(PageBoxPageItem* pageItem1, PageBoxPageItem* pageItem2 = nullptr);

    void onSizeChanged(QSizeF const & size);

    void onTransformChanged();

    void onCurrentPageChanged();

private:
    QAbstractItemModel * model_;
    ResourceTransform * transform_;

    QSizeF pageSize_;
    QSizeF pageSize1_;
    QSizeF pageSize2_;
    Direction direction_;
    LayoutMode layoutMode_;
    ScaleMode scaleMode_;
    qreal padding_; // Continuous mode
    qreal manualScale_;
    QPointF pos_;
    int curPage_;
    qreal scaleInterval_;
    int scaleLevel_;
    int maxScaleLevel_;

private:
    QGraphicsRectItem * pageCanvas_;
    PageBoxPlugin * plugin_;
    QGraphicsItem * pluginItem_;

private:
    QPropertyBindings * itemBindings_;
};

#endif // PAGEBOXDOCITEM_H
