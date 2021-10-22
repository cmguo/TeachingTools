#ifndef INKSTROKEGEOMETRY_H
#define INKSTROKEGEOMETRY_H

#include <InkCanvas_global.h>

#include <QSharedPointer>

#include <array>

INKCANVAS_FORWARD_DECLARE_CLASS(InkCanvas);
INKCANVAS_FORWARD_DECLARE_CLASS(Stroke);
INKCANVAS_FORWARD_DECLARE_CLASS(InkCanvasStrokesReplacedEventArgs);
INKCANVAS_FORWARD_DECLARE_CLASS(InkCanvasStrokeCollectedEventArgs);
INKCANVAS_FORWARD_DECLARE_CLASS(StrokeCollection);

class Geometry;

class InkStrokeGeometry : public QObject
{
    Q_OBJECT
public:
    enum Shape
    {
        None,
        Line,
        Wavy,
        Auto,
    };

    Q_ENUM(Shape)

    static std::array<Shape, 4> Shapes;

public:
    void reshape(QSharedPointer<Stroke> stroke, Shape shape);

    static void applyPressure(QSharedPointer<Stroke> stroke);

    static void toLine(QSharedPointer<Stroke> stroke);

    static void toCurve(QSharedPointer<Stroke> stroke, qreal(func)(qreal), qreal scale);

    static void autoShape(QSharedPointer<Stroke> stroke, QSharedPointer<StrokeCollection> & allStrokes, void * & autoGeometry);

public:
    InkStrokeGeometry(InkCanvas* ink);

    ~InkStrokeGeometry();

public:
    Shape shapeMode() const { return shapeMode_; }

    void setShapeMode(Shape shape);

private:
    void strokesReplaced(InkCanvasStrokesReplacedEventArgs &e);

    void applyShape(InkCanvasStrokeCollectedEventArgs &e);

private:
    void timerEvent(QTimerEvent *event) override;

private:
    InkCanvas * ink_;
    Shape shapeMode_;
    QSharedPointer<StrokeCollection> autoStrokes_;
    void * autoGeometry_ = nullptr;
};

#endif // INKSTROKEGEOMETRY_H
