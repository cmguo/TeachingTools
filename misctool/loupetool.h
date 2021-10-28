#ifndef LOUPECONTROL_H
#define LOUPECONTROL_H

#include <core/control.h>

class LoupeTool : public Control
{
    Q_OBJECT
public:
    Q_INVOKABLE explicit LoupeTool(ResourceView *res);

protected:
    ControlView * create(ControlView *parent) override;

    void attached() override;

    SelectMode selectTest(ControlView *child, ControlView *parent, const QPointF &point, bool onlyAssist) override;
};

#endif // LOUPECONTROL_H
