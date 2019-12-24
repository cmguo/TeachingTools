#ifndef WRITINGGRIDCONTROL_H
#define WRITINGGRIDCONTROL_H

#include "TeachingTools_global.h"

#include <core/control.h>
#include <QGraphicsPixmapItem>
#include <qexport.h>

class TEACHINGTOOLS_EXPORT WritingGridControl: public Control
{
    Q_OBJECT

public:
    Q_INVOKABLE WritingGridControl(ResourceView *res);

    virtual QString toolsString(QString const & parent = QString()) const override;

    using :: Control::sizeChanged;
private slots:
    void changeGridType();

protected:
    virtual QGraphicsItem * create(ResourceView * res) override;

    virtual void attaching() override;

    virtual void attached() override;
};

#endif // WRITINGGRIDCONTROL_H
