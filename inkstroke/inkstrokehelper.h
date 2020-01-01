#ifndef INKSTROKEHELPER_H
#define INKSTROKEHELPER_H

#include <core/control.h>

class ToolButton;
class InkCanvas;
class InkCanvasStrokeCollectedEventArgs;

class InkStrokeHelper
{
public:
    static InkCanvas* createInkCanvas(qreal lineWidth = 4.0);

    static Control::SelectMode selectTest(InkCanvas* ink, QPointF const & pt, bool eatUnselect = true);

    static QString toolString();

    static void updateToolButton(InkCanvas* ink, ToolButton * button);

    static void getToolButtons(InkCanvas* ink, QList<ToolButton *> & buttons,
                                ToolButton * parent);

private:
    static QWidget * createEraserWidget();
};

#endif // INKSTROKEHELPER_H
