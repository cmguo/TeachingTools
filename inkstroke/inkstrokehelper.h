#ifndef INKSTROKEHELPER_H
#define INKSTROKEHELPER_H

#include "TeachingTools_global.h"

#include <core/control.h>

class ToolButton;
class InkCanvas;
class InkCanvasStrokeCollectedEventArgs;
class QssHelper;

#include <QColor>

#define MIX_SELECT 1

class TEACHINGTOOLS_EXPORT InkStrokeHelper
{
public:
    static InkCanvas* createInkCanvas(QColor color = QColor("#FFF0F0F0"),
                                      qreal lineWidth = 4.0,
                                      QSizeF eraserSize = {48, 80}); // unit inch or pixel ( w >= 10)

    static Control::SelectMode selectTest(InkCanvas* ink, QPointF const & pt,
                                          bool eatUnselect = true, bool mixSelect = false);

    static QString toolString();

    static bool updateToolButton(InkCanvas* ink, ToolButton * button);

    static void getToolButtons(InkCanvas* ink, QList<ToolButton *> & buttons,
                                ToolButton * parent);

public:
    static QWidget * createEraserWidget(QssHelper const & qss);
};

#endif // INKSTROKEHELPER_H
