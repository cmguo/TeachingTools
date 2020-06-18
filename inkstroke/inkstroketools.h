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
    Q_PROPERTY(bool enabled READ enabled WRITE setEnabled)
    Q_PROPERTY(bool disabled READ disabled WRITE setDisabled)
    Q_PROPERTY(QColor color READ color WRITE setColor)
    Q_PROPERTY(qreal width READ width WRITE setWidth)

public:
    InkStrokeTools(QObject* parent = nullptr, WhiteCanvas* whiteCanvas = nullptr);

    virtual ~InkStrokeTools() override;

    static InkStrokeTools * instance();

public:
    void attachToWhiteCanvas(WhiteCanvas* whiteCanvas);

    Q_INVOKABLE void setOuterControl(QObject* control, bool sync = false);

public:
    InkCanvasEditingMode mode() const { return mode_; }

    void setMode(InkCanvasEditingMode mode);

    bool disabled() const { return disabled_; }

    void setDisabled(bool b);

    bool enabled() const { return !disabled_; }

    void setEnabled(bool b);

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
    Q_DISABLE_COPY(InkStrokeTools)

    void switchPage(ResourcePage * page);

    void togglePopupMenu(ToolButton *button);

    QWidget* createWidget(ToolButton *button);

    QWidget * createPenWidget(ToolButton *button);

    QWidget * createEraserWidget(ToolButton *button);

private:
    friend class SyncInkControl;
    WhiteCanvas * canvas_;
    InkStrokeControl * inkControl_;
    QObject * outerControl_;
    QObject * activeControl_;
    InkCanvasEditingMode mode_;
    bool disabled_ = false;
    QColor colorNormal_;
    QColor colorShow_;
    QColor colorOuter_;
    QColor* inkColor_;
    QColor* activeColor_;
    qreal width_;
};

class SyncInkControl : public QObject
{
    Q_OBJECT
    Q_PROPERTY(int editingMode READ editingMode WRITE setEditingMode)
    Q_PROPERTY(QColor color WRITE setColor)
    Q_PROPERTY(qreal width WRITE setWidth)
public:
    SyncInkControl(InkStrokeTools * tools, QObject * outerControl);

protected:
    void setEditingMode(int mode);

    int editingMode() const;

    void clear();

    void setWidth(qreal width);

    void setColor(const QColor &color);

private:
    friend class InkStrokeTools;
    InkStrokeTools * tools_;
    QObject * outerControl_;
};

#endif // INKSTROKETOOLS_H
