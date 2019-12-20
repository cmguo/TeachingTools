#ifndef PAGEBOXCONTROL_H
#define PAGEBOXCONTROL_H

#include "TeachingTools_global.h"

#include <core/control.h>

class PageBoxItem;

class TEACHINGTOOLS_EXPORT PageBoxControl : public Control
{
    Q_OBJECT

    Q_PROPERTY(QByteArray pageBoxState READ pageBoxState  WRITE setPageBoxState)
public:
    Q_INVOKABLE PageBoxControl(ResourceView * res);

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

    void enableInkPad();

protected:
    QByteArray pageBoxState();

    void setPageBoxState(QByteArray state);

    void loadPages();

private:
    void loadPages(PageBoxItem * item);
};

#endif // PAGEBOXCONTROL_H
