#ifndef CAPTURETOOL_H
#define CAPTURETOOL_H

#include <core/control.h>

class CaptureTool : Control
{
    Q_OBJECT
public:
    Q_INVOKABLE CaptureTool(ResourceView * res);

private:
    ControlView * create(ControlView *parent) override;

    void resize(const QSizeF &size) override;
};

#endif // CAPTURETOOL_H
