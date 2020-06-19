#ifndef INKSTROKECONTROL_H
#define INKSTROKECONTROL_H

#include "TeachingTools_global.h"

#include <core/control.h>
#include <Windows/Controls/editingmode.h>

#include <QColor>
#include <QPolygonF>

class InkCanvas;
class InkCanvasStrokeCollectedEventArgs;
class InkCanvasStrokeErasingEventArgs;

class InkStrokeControl : public Control
{
    Q_OBJECT

    Q_PROPERTY(InkCanvasEditingMode editingMode READ editingMode WRITE setEditingMode NOTIFY editingModeChanged)
    Q_PROPERTY(QColor color READ color WRITE setColor)
    Q_PROPERTY(qreal width READ width WRITE setWidth)

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

signals:
    void editingModeChanged(InkCanvasEditingMode mode);

protected:
    virtual QGraphicsItem * create(ResourceView *res) override;

    virtual void attached() override;

    virtual void resize(const QSizeF &size) override;

    virtual SelectMode selectTest(QPointF const & pt) override;

    virtual void detaching() override;

private:
    void setupMultiLayerErasing();

    void teardownMultiLayerErasing();

private:
    QGraphicsItem * filterItem_;
};

#endif // INKSTROKECONTROL_H
