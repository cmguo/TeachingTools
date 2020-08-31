#ifndef PAGEBOXCONTROL_H
#define PAGEBOXCONTROL_H

#include "TeachingTools_global.h"

#include <core/control.h>

class PageBoxItem;

class TEACHINGTOOLS_EXPORT PageBoxControl : public Control
{
    Q_OBJECT
public:
    Q_INVOKABLE PageBoxControl(ResourceView * res, Flags flags = {}, Flags clearFlags = {});

    using Control::sizeChanged;

protected:
    virtual QGraphicsItem * create(ResourceView *res) override;

    virtual void attaching() override;

    virtual void attached() override;

    virtual void detaching() override;

    virtual void resize(QSizeF const & size) override;

    virtual SelectMode selectTest(QPointF const & point) override;

protected:
    virtual void loadData();

    virtual void parseData();

    virtual QSizeF pageSize();

    void enableInkPad();

protected:
    void loadPages(int initialPage = 0);

private:
    void loadPages(PageBoxItem * item);

private:
    QGraphicsTransform * bottomTransform_;
};

#endif // PAGEBOXCONTROL_H
