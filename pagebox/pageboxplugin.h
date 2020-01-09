#ifndef PAGEBOXPLUGIN_H
#define PAGEBOXPLUGIN_H

#include "TeachingTools_global.h"

#include <core/toolbuttonprovider.h>

#include <QObject>

class QGraphicsItem;
class PageBoxDocItem;
class ToolButton;

class TEACHINGTOOLS_EXPORT PageBoxPlugin : public ToolButtonProvider
{
    Q_OBJECT
public:
    PageBoxPlugin(QObject * parent = nullptr);

    virtual ~PageBoxPlugin();

public:
    QGraphicsItem * item() { return item_; }

    virtual void onRelayout(int pageCount, int curPage);

    virtual void onPageChanged(int lastPage, int curPage);

    virtual void onSizeChanged(QSizeF const & docSize, QSizeF const & pageSize,
                               QSizeF const & viewSize);

    virtual bool selectTest(QPointF const & pt);

    virtual PageBoxPlugin * clone() const ;

protected:
    PageBoxDocItem *document();

protected:
    QGraphicsItem * item_;
};

#endif // PAGEBOXPLUGIN_H
