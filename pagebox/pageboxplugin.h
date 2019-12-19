#ifndef PAGEBOXPLUGIN_H
#define PAGEBOXPLUGIN_H

#include "TeachingTools_global.h"

#include <QObject>

class QGraphicsItem;
class PageBoxDocItem;

class TEACHINGTOOLS_EXPORT PageBoxPlugin : public QObject
{
    Q_OBJECT
public:
    PageBoxPlugin();

    virtual ~PageBoxPlugin();

public:
    QGraphicsItem * item() { return item_; }

    virtual void onRelayout(int pageCount, int curPage);

    virtual void onPageChanged(int lastPage, int curPage);

    virtual void onSizeChanged(QSizeF const & docSize, QSizeF const & pageSize,
                               QSizeF const & viewSize);

protected:
    PageBoxDocItem *document();

protected:
    QGraphicsItem * item_;
};

#endif // PAGEBOXPLUGIN_H
