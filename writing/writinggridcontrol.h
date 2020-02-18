#ifndef WRITINGGRIDCONTROL_H
#define WRITINGGRIDCONTROL_H

#include "TeachingTools_global.h"

#include <core/control.h>
#include <qexport.h>

#include <QGraphicsPixmapItem>
#include <QSharedPointer>

class StrokeCollection;

class TEACHINGTOOLS_EXPORT WritingGridControl: public Control
{
    Q_OBJECT
public:
    Q_INVOKABLE WritingGridControl(ResourceView *res);

    using :: Control::sizeChanged;

private slots:
    void changeGridType();

protected:
    virtual QGraphicsItem * create(ResourceView * res) override;

    virtual void attaching() override;

    virtual void attached() override;

    virtual void afterClone(Control * control) override;
};

#endif // WRITINGGRIDCONTROL_H
