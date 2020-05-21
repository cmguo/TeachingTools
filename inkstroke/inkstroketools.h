#ifndef INKSTROKETOOLS_H
#define INKSTROKETOOLS_H

#include "TeachingTools_global.h"

#include <core/toolbuttonprovider.h>

#include <Windows/Controls/editingmode.h>

#include <QColor>

class WhiteCanvas;
class Control;
class InkStrokeControl;
class ResourcePage;

class TEACHINGTOOLS_EXPORT InkStrokeTools : public ToolButtonProvider
{
    Q_OBJECT

    Q_PROPERTY(InkCanvasEditingMode mode READ mode WRITE setMode)
    Q_PROPERTY(QColor color READ color WRITE setColor)
    Q_PROPERTY(qreal width READ width WRITE setWidth)

public:
    InkStrokeTools(QObject* parent = nullptr, WhiteCanvas* whiteCanvas = nullptr);

public:
    void attachToWhiteCanvas(WhiteCanvas* whiteCanvas);

    Q_INVOKABLE void setOuterControl(QObject* control);

public:
    InkCanvasEditingMode mode() const { return mode_; }

    void setMode(InkCanvasEditingMode mode);

    QColor color() { return *activeColor_; }

    void setColor(QColor color);

    qreal width() { return width_; }

    void setWidth(qreal width);

    void clearInkStroke();

public:
    using ToolButtonProvider::getToolButtons;

    virtual void getToolButtons(QList<ToolButton *> &buttons, ToolButton *parent) override;

    virtual void updateToolButton(ToolButton *button) override;

protected:
    virtual bool setOption(const QByteArray &key, QVariant value) override;

private:
    void switchPage(ResourcePage * page);

    void togglePopupMenu(ToolButton *button);

    QWidget* createWidget(ToolButton *button);

    QWidget * createPenWidget(ToolButton *button);

    QWidget * createEraserWidget(ToolButton *button);

private:
    WhiteCanvas * canvas_;
    InkStrokeControl * inkControl_;
    QObject * outerControl_;
    QObject * activeControl_;
    InkCanvasEditingMode mode_;
    bool disable_ = false;
    QColor colorNormal_;
    QColor colorShow_;
    QColor colorOuter_;
    QColor* inkColor_;
    QColor* activeColor_;
    qreal width_;
};

#endif // INKSTROKETOOLS_H
