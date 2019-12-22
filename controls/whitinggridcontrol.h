#ifndef WHITINGGRIDCONTROL_H
#define WHITINGGRIDCONTROL_H

#include "TeachingTools_global.h"

#include <core/control.h>
#include <QGraphicsPixmapItem>
#include <qexport.h>

class TEACHINGTOOLS_EXPORT WhitingGridControl: public Control
{
    Q_OBJECT

public:
    Q_INVOKABLE WhitingGridControl(ResourceView *res);

    virtual QString toolsString(QString const & parent = QString()) const override;

    using :: Control::sizeChanged;
private slots:
    void changeGridType();

protected:
    virtual QGraphicsItem * create(ResourceView * res) override;

    virtual void attaching() override;

    virtual void attached() override;
};

#endif // WHITINGGRIDCONTROL_H
