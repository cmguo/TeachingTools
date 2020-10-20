#include "inkstrokegeometry.h"

#include <geometry.h>
#include <geometryhelper.h>
#include <views/qsshelper.h>

#include <qcomponentcontainer.h>

#include <Windows/Ink/stroke.h>

#include <Landing/Qt/inkcanvasqt.h>

#include <Windows/Controls/inkcanvas.h>
#include <Windows/Controls/inkevents.h>

#include <Windows/Input/stylusdevice.h>

INKCANVAS_USE_NAMESPACE

void InkStrokeGeometry::reshape(QSharedPointer<Stroke> stroke, Shape shape)
{
    switch (shape) {
    case None:
        //applyPressure(stroke);
        break;
    case Line:
        toLine(stroke);
        break;
    case Wavy:
        toCurve(stroke, sin, dp(4.0));
        break;
    case Auto:
        autoShape(stroke);
        break;
    }
}

void InkStrokeGeometry::applyPressure(QSharedPointer<Stroke> stroke)
{
    QSharedPointer<StylusPointCollection> stylusPoints = stroke->StylusPoints()->Clone();
    int n = 16;
    if (stylusPoints->Count() > n) {
        for (int i = 1; i < n; ++i) {
            int m = stylusPoints->Count() + i - n;
            StylusPoint point = (*stylusPoints)[m];
            float d = static_cast<float>(i) / static_cast<float>(n);
            point.SetPressureFactor(point.PressureFactor() * (1.0f - d * d));
            stylusPoints->SetItem(m, point);
        }
        --n;
    } else {
        n = 0;
    }
    //QUuid guid("52053C24-CBDD-4547-AAA1-DEFEBF7FD1E1");
    //stroke->AddPropertyData(guid, 2.0);
    stroke->SetStylusPoints(stylusPoints);
}

void InkStrokeGeometry::toLine(QSharedPointer<Stroke> stroke)
{
    StylusPointCollection & stylusPoints = *stroke->StylusPoints();
    int n = stylusPoints.Count();
    if (n <= 2)
        return;
    QSharedPointer<StylusPointCollection> points(
            new StylusPointCollection(stroke->StylusPoints()->Description(), 2));
    points->Add(stylusPoints[0]);
    points->Add(stylusPoints[n - 1]);
    stroke->SetStylusPoints(points);
}

void InkStrokeGeometry::toCurve(QSharedPointer<Stroke> stroke, qreal(func)(qreal), qreal scale)
{
    StylusPointCollection & stylusPoints = *stroke->StylusPoints();
    int n = stylusPoints.Count();
    if (n <= 2)
        return;
    Point s = stylusPoints[0].ToPoint();
    Point e = stylusPoints[n - 1].ToPoint();
    qreal l = (e - s).Length();
    QSharedPointer<StylusPointCollection> points(
            new StylusPointCollection(stroke->StylusPoints()->Description(), 2));
    qreal x = 0.0;
    qreal scale_ = 1.0 / scale;
    while (true) {
        bool f = x >= l;
        if (f)
            x = l;
        qreal y = func(x * scale_) * scale;
        StylusPoint p(x, -y);
        points->Add(p);
        if (f)
            break;
        x += scale;
    }
    stroke->SetStylusPoints(points);
    Matrix m;
    m.Rotate(-GeometryHelper::angle(Point(e - s)));
    m.Translate(s.X(), s.Y());
    stroke->Transform(m, false);
}

void InkStrokeGeometry::autoShape(QSharedPointer<Stroke> stroke)
{
    (void) stroke;
}

QSharedPointer<Stroke> InkStrokeGeometry::fromGeometry(::Geometry *geometry)
{
    QSharedPointer<Stroke> s = InkCanvasQt::createStroke(
        geometry->graphPath(), geometry->color(), geometry->width());
    return s;
}

InkStrokeGeometry::InkStrokeGeometry(InkCanvas *ink)
    : QObject(ink)
    , ink_(ink)
{
    ink_->AddHandler(InkCanvas::StrokeCollectedEvent, RoutedEventHandlerT<
                     InkStrokeGeometry, InkCanvasStrokeCollectedEventArgs, &InkStrokeGeometry::applyShape>(this));
    QObject::connect(ink_, &InkCanvas::StrokesReplaced,
                     this, &InkStrokeGeometry::strokesReplaced);
}

InkStrokeGeometry::~InkStrokeGeometry()
{
    // InkCanvas already destoryed
    //ink_->RemoveHandler(InkCanvas::StrokeCollectedEvent, RoutedEventHandlerT<
    //                InkStrokeGeometry, InkCanvasStrokeCollectedEventArgs, &InkStrokeGeometry::applyPressure>(this));
}

void InkStrokeGeometry::setShapeMode(Shape shape)
{
    shapeMode_ = shape;
}

void InkStrokeGeometry::strokesReplaced(InkCanvasStrokesReplacedEventArgs &e)
{
    (void) e;
    ink_->SetDefaultStylusPointDescription(Stylus::DefaultPointDescription());
}

void InkStrokeGeometry::applyShape(InkCanvasStrokeCollectedEventArgs &e)
{
    QSharedPointer<Stroke> stroke = e.GetStroke();
    reshape(stroke, shapeMode_);
}
