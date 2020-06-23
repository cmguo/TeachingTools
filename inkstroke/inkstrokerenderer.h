#ifndef INKSTROKERENDERER_H
#define INKSTROKERENDERER_H

#include <stroke/strokesrenderer.h>

#include <Windows/Ink/drawingattributes.h>
#include <Windows/Ink/strokecollection.h>
#include <Windows/Input/styluspointcollection.h>

INKCANVAS_FORWARD_DECLARE_CLASS(StrokeCollection);
INKCANVAS_FORWARD_DECLARE_CLASS(DrawingAttributes);
INKCANVAS_FORWARD_DECLARE_CLASS(StylusPointCollection);

class InkStrokeRenderer : public StrokesRenderer
{
    Q_OBJECT
public:
    InkStrokeRenderer(StrokesReader* reader, QSizeF const & maxSize, QSharedPointer<StrokeCollection> strokes,
                      QSharedPointer<DrawingAttributes> attr, QObject * parent);

public:
    QSizeF size() const;

    // IDynamicRenderer interface
protected:
    virtual void setMaximun(const StrokePoint &max) override;

    virtual void startStroke(const StrokePoint &point) override;

    virtual void addPoint(const StrokePoint &pt) override;

    virtual void endStroke() override;

private:
    QSharedPointer<StrokeCollection> strokes_;
    QSharedPointer<DrawingAttributes> da_;
    QSharedPointer<StylusPointCollection> stylusPoints_;
    QSizeF destSize_;
    qreal scale_;
    float pressureScale_;
};

#endif // INKSTROKERENDERER_H
