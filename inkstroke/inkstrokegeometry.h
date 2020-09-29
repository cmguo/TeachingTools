#ifndef INKSTROKEGEOMETRY_H
#define INKSTROKEGEOMETRY_H

#include <InkCanvas_global.h>

#include <QSharedPointer>
#include <array>

INKCANVAS_FORWARD_DECLARE_CLASS(InkCanvas);
INKCANVAS_FORWARD_DECLARE_CLASS(Stroke);
INKCANVAS_FORWARD_DECLARE_CLASS(InkCanvasStrokesReplacedEventArgs);
INKCANVAS_FORWARD_DECLARE_CLASS(InkCanvasStrokeCollectedEventArgs);

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

    static constexpr std::array<Shape, 4> Shapes {None, Line, Wavy, Auto};

public:
    static void reshape(QSharedPointer<Stroke> stroke, Shape shape);

    static void applyPressure(QSharedPointer<Stroke> stroke);

    static void toLine(QSharedPointer<Stroke> stroke);

    static void toCurve(QSharedPointer<Stroke> stroke, qreal(func)(qreal), qreal scale);

    static void autoShape(QSharedPointer<Stroke> stroke);

    static QSharedPointer<Stroke> fromGeometry(::Geometry * geometry);

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
    InkCanvas * ink_;
    Shape shapeMode_;
};

#endif // INKSTROKEGEOMETRY_H
