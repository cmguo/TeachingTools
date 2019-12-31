#ifndef INKSTROKECONTROL_H
#define INKSTROKECONTROL_H

#include "TeachingTools_global.h"

#include <core/widgetcontrol.h>
#include <Windows/Controls/editingmode.h>

#include <QColor>

class InkCanvas;
class InkCanvasStrokeCollectedEventArgs;

class TEACHINGTOOLS_EXPORT InkStrokeControl : public Control
{
    Q_OBJECT

    Q_PROPERTY(InkCanvasEditingMode editingMode READ editingMode WRITE setEditingMode)
    Q_PROPERTY(QColor color READ color WRITE setColor)
    Q_PROPERTY(qreal width READ width WRITE setWidth)

public:
    static InkCanvas* createInkCanvas(qreal lineWidth = 4.0);

    static SelectMode selectTest(InkCanvas* ink, QPointF const & pt, bool eatUnselect = true);

    static ToolButton* createEraserButton();

public:
    Q_INVOKABLE InkStrokeControl(ResourceView* res);

    InkCanvasEditingMode editingMode();

    QColor color();

    qreal width();

public slots:
    void setEditingMode(InkCanvasEditingMode mode);

    void setColor(QColor c);

    void setWidth(qreal w);

    void clear();

protected:
    virtual QGraphicsItem * create(ResourceView *res) override;

    virtual void attached() override;

    virtual void resize(const QSizeF &size) override;

    virtual SelectMode selectTest(QPointF const & pt) override;

private:
    static QWidget * createEraserWidget();
};

#endif // INKSTROKECONTROL_H
