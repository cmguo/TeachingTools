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

    virtual ~PageBoxPlugin() override;

public:
    QGraphicsItem * item() { return item_; }

    virtual void onSizeChanged(QSizeF const & docSize, QSizeF const & pageSize);

    virtual void onRelayout(int pageCount, int curPage);

    virtual void onPageChanged(int lastPage, int curPage);

    virtual bool selectTest(QPointF const & pt);

    virtual PageBoxPlugin * clone() const ;

protected:
    PageBoxDocItem *document();

protected:
    QGraphicsItem * item_;

private:
    Q_DISABLE_COPY(PageBoxPlugin)
};

#endif // PAGEBOXPLUGIN_H
