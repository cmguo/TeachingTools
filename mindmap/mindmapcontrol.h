#ifndef MINDNODECONTROL_H
#define MINDNODECONTROL_H

#include <core/control.h>

class MindMapControl : public Control
{
    Q_OBJECT
public:
    Q_INVOKABLE MindMapControl(ResourceView * res);

    using :: Control::sizeChanged;

protected:
    ControlView * create(ControlView *parent) override;

    void attached() override;

    void onData(QByteArray data) override;
};

#endif // MINDNODECONTROL_H
