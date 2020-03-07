#ifndef INKSTROKETOOLS_H
#define INKSTROKETOOLS_H

#include "TeachingTools_global.h"

#include <core/toolbuttonprovider.h>

#include <Windows/Controls/editingmode.h>

#include <QColor>

class WhiteCanvas;
class Control;
class InkStrokeControl;

class TEACHINGTOOLS_EXPORT InkStrokeTools : public ToolButtonProvider
{
    Q_OBJECT

    Q_PROPERTY(InkCanvasEditingMode mode READ mode WRITE setMode)
    Q_PROPERTY(QColor color READ color WRITE setColor)
    Q_PROPERTY(qreal width READ width WRITE setWidth)

public:
    InkStrokeTools(WhiteCanvas* whiteCanvas = nullptr);

public:
    void attachToWhiteCanvas(WhiteCanvas* whiteCanvas);

    void setOuterControl(Control* control);

public:
    InkCanvasEditingMode mode() const { return mode_; }

    void setMode(InkCanvasEditingMode mode);

    QColor color() { return *activeColor_; }

    void setColor(QColor color);

    qreal width() { return width_; }

    void setWidth(qreal width);

    void clearInkStroke();

protected:
    virtual void getToolButtons(QList<ToolButton *> &buttons, ToolButton *parent) override;

    virtual void updateToolButton(ToolButton *button) override;

    virtual void setOption(const QByteArray &key, QVariant value) override;

    virtual bool eventFilter(QObject *, QEvent *event) override;

private:
    void togglePopupMenu(ToolButton *button);

    QWidget* createWidget(ToolButton *button);

    QWidget * createPenWidget(ToolButton *button);

    QWidget * createEraserWidget(ToolButton *button);

private:
    WhiteCanvas * canvas_;
    InkStrokeControl * inkControl_;
    Control * outerControl_;
    InkCanvasEditingMode mode_;
    QColor colorNormal_;
    QColor colorShow_;
    QColor colorOuter_;
    QColor* activeColor_;
    Control * activeControl_;
    qreal width_;
};

#endif // INKSTROKETOOLS_H
