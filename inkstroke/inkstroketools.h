#ifndef INKSTROKETOOLS_H
#define INKSTROKETOOLS_H

#include <core/toolbuttonprovider.h>

#include <Windows/Controls/editingmode.h>

#include <QColor>

class WhiteCanvas;
class Control;

class InkStrokeTools : ToolButtonProvider
{
public:
    InkStrokeTools();

public:
    void attachToWhiteCanvas(WhiteCanvas* whiteCanvas);

    void setOuterControl(Control* control);

protected:
    void setColor(QColor color);

    void setWidth(qreal width);

protected:
    virtual void handleToolButton(ToolButton *button, QStringList const & args) override;

private:
    void togglePopupMenu(ToolButton *button);

private:
    WhiteCanvas * canvas_;
    Control * outerControl_;
    InkCanvasEditingMode mode_;
    QColor colorNormal_;
    QColor colorShow_;
    QColor colorOuter_;
    QColor* activeColor_;
    qreal width_;
};

#endif // INKSTROKETOOLS_H
