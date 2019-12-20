#ifndef INKSTROKECONTROL_H
#define INKSTROKECONTROL_H

#include "TeachingTools_global.h"

#include <core/widgetcontrol.h>
#include <Windows/Controls/editingmode.h>

class InkCanvas;
class InkCanvasStrokeCollectedEventArgs;

class TEACHINGTOOLS_EXPORT InkStrokeControl : public WidgetControl
{
    Q_OBJECT

    Q_PROPERTY(InkCanvasEditingMode editingMode READ editingMode WRITE setEditingMode)

public:
    static InkCanvas* createInkCanvas(qreal lineWidth = 4.0);

    static void applyPressure(InkCanvasStrokeCollectedEventArgs& e);

public:
    Q_INVOKABLE InkStrokeControl(ResourceView* res);

    InkCanvasEditingMode editingMode();

public slots:
    void setEditingMode(InkCanvasEditingMode mode);

protected:
    virtual QWidget *createWidget(ResourceView *res) override;

    virtual void attached() override;

    virtual void detaching() override;

    virtual SelectMode selectTest(QPointF const & pt) override;

private:
    void onStrokeCollected(InkCanvasStrokeCollectedEventArgs& e);
};

#endif // INKSTROKECONTROL_H
