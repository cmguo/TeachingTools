#ifndef SHOWBOARDCONTROL_H
#define SHOWBOARDCONTROL_H

#include <core/control.h>
#include <QGraphicsPixmapItem>
#include <qexport.h>
#include "TeachingTools_global.h"

class TEACHINGTOOLS_EXPORT ShowBoardControl: public Control
{
    Q_OBJECT

public:
    Q_INVOKABLE ShowBoardControl(ResourceView *res);

    virtual QString toolsString(QString const & parent = QString()) const override;
private slots:
    void changeGridType();

protected:
    virtual QGraphicsItem * create(ResourceView * res) override;

    virtual void attaching() override;

    virtual void attached() override;
};

#endif // SHOWBOARDCONTROL_H
