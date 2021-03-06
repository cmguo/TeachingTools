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

public slots:
    bool next();

    bool prev();

    void switchPage(int);

protected:
    virtual ControlView * create(ControlView * parent) override;

    virtual void attaching() override;

    virtual void attached() override;

    virtual void detaching() override;

    virtual void resize(QSizeF const & size) override;

    virtual SelectMode selectTest(QPointF const & point) override;


protected:
    virtual bool handleToolButton(ToolButton * button, QStringList const & args) override;

    virtual bool event(QEvent *event) override;

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
    ControlTransform * bottomTransform_;
};

#endif // PAGEBOXCONTROL_H
