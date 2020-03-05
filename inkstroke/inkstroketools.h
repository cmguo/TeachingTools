#ifndef INKSTROKETOOLS_H
#define INKSTROKETOOLS_H

#include "TeachingTools_global.h"

#include <core/toolbuttonprovider.h>

#include <Windows/Controls/editingmode.h>

#include <QColor>

class WhiteCanvas;
class Control;

class TEACHINGTOOLS_EXPORT InkStrokeTools : ToolButtonProvider
{
    Q_OBJECT

    Q_PROPERTY(QColor color READ color WRITE setColor)
public:
    InkStrokeTools();

public:
    void attachToWhiteCanvas(WhiteCanvas* whiteCanvas);

    void setOuterControl(Control* control);

protected:
    QColor color() { return *activeColor_; }

    qreal width() { return width_; }

    void setColor(QColor color);

    void setWidth(qreal width);

protected:
    virtual void handleToolButton(ToolButton *button, QStringList const & args) override;

private:
    void togglePopupMenu(ToolButton *button);

public:
    static QWidget* createWidget(QObject* inkControl, QByteArray const & name);

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
