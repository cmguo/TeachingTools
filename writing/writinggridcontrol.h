#ifndef WRITINGGRIDCONTROL_H
#define WRITINGGRIDCONTROL_H

#include "TeachingTools_global.h"

#include <core/control.h>
#include <InkCanvas_global.h>

#include <qexport.h>

#include <QGraphicsPixmapItem>
#include <QSharedPointer>

INKCANVAS_BEGIN_NAMESPACE
class StrokeCollection;
INKCANVAS_END_NAMESPACE

class TEACHINGTOOLS_EXPORT WritingGridControl: public Control
{
    Q_OBJECT
public:
    Q_INVOKABLE WritingGridControl(ResourceView *res);

    using Control::sizeChanged;

private slots:
    void changeGridType();

protected:
    virtual QGraphicsItem * create(ResourceView * res) override;

    virtual void attaching() override;

    virtual void attached() override;

    virtual void afterClone(Control * control) override;

    virtual void copy(QMimeData &data) override;

private:
    float m_adapterRatio = 1.0f;
};

#endif // WRITINGGRIDCONTROL_H
