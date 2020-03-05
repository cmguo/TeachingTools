#ifndef INKSTROKEHELPER_H
#define INKSTROKEHELPER_H

#include <core/control.h>

class ToolButton;
class InkCanvas;
class InkCanvasStrokeCollectedEventArgs;

#include <QColor>

#define MIX_SELECT 1

class InkStrokeHelper
{
public:
    static InkCanvas* createInkCanvas(QColor color = QColor("#FFF0F0F0"), qreal lineWidth = 4.0, QSizeF eraserSize = {48, 80});

    static Control::SelectMode selectTest(InkCanvas* ink, QPointF const & pt, bool eatUnselect = true, bool mixSelect = false);

    static QString toolString();

    static void updateToolButton(InkCanvas* ink, ToolButton * button);

    static void getToolButtons(InkCanvas* ink, QList<ToolButton *> & buttons,
                                ToolButton * parent);

public:
    static QWidget * createPenWidget(QColor color, qreal width);

    static QWidget * createEraserWidget();
};

#endif // INKSTROKEHELPER_H
