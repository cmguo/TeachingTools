#include "inkstrokegeometry.h"

#include <widget/qsshelper.h>
#include <showboard.h>
#include <core/control.h>
#include <core/resourceview.h>
#include <core/resourcepage.h>

#include <qcomponentcontainer.h>

#include <Windows/Ink/stroke.h>
#include <Landing/Qt/inkcanvasqt.h>
#include <Windows/Controls/inkcanvas.h>
#include <Windows/Controls/inkevents.h>
#include <Windows/Input/stylusdevice.h>

INKCANVAS_USE_NAMESPACE

std::array<InkStrokeGeometry::Shape, 4> InkStrokeGeometry::Shapes = {None, Line, Wavy, Auto};

static QObject * geometryHelper()
{
    static QObject* geometryHelper = QComponentContainer::globalInstance().getExportValue("GeometryHelper");
    return geometryHelper;
}

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
        if (autoGeometry_)
            killTimer(0);
        autoShape(stroke, autoStrokes_, autoGeometry_);
        if (autoGeometry_)
            startTimer(2000);
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
        StylusPoint p = stylusPoints[0];
        p.SetX(x);
        p.SetY(-y);
        points->Add(p);
        if (f)
            break;
        x += scale;
    }
    stroke->SetStylusPoints(points);
    Matrix m;
    QObject * geometryHelper = ::geometryHelper();
    if (geometryHelper) {
        qreal angle = 0;
        bool ok = QMetaObject::invokeMethod(geometryHelper, "angle", Q_RETURN_ARG(qreal,angle),
                      Q_ARG(QPointF,QPointF(Point(e - s))));
        if (ok)
            m.Rotate(-angle);
    }
    m.Translate(s.X(), s.Y());
    stroke->Transform(m, false);
}

static qreal length2(QPointF const & p1, QPointF const & p2)
{
    auto d = p2 - p1;
    return QPointF::dotProduct(d, d);
}

void InkStrokeGeometry::autoShape(QSharedPointer<Stroke> stroke, QSharedPointer<StrokeCollection> & allStrokes, void * & autoGeometry)
{
    if (stroke->StylusPoints()->Count() < 2)
        return;
    QObject * geometryHelper = ::geometryHelper();
    if (geometryHelper == nullptr)
        return;
    if (autoGeometry == nullptr) {
        bool ok = QMetaObject::invokeMethod(geometryHelper, "approxGeometryBegin", Q_RETURN_ARG(void*,autoGeometry));
        if (!ok || autoGeometry == nullptr)
            return;
    }
    StylusPointCollection & stylusPoints = *stroke->StylusPoints();
    QVector<QPointF> points;
    for (auto & p : stylusPoints) {
        points.append(p.ToPoint());
    }
    bool result;
    bool ok = QMetaObject::invokeMethod(geometryHelper, "approxGeometryAddPoints", Q_RETURN_ARG(bool,result),
                                        Q_ARG(void*,autoGeometry),
                                        Q_ARG(QVector<QPointF>,points));
    if (ok) {
        if (!allStrokes)
            allStrokes.reset(new StrokeCollection);
        if (!result)
            allStrokes->Clear();
        allStrokes->Add(stroke);
    }
}

InkStrokeGeometry::InkStrokeGeometry(InkCanvas *ink)
    : QObject(ink)
    , ink_(ink)
    , shapeMode_(Auto)
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

void InkStrokeGeometry::timerEvent(QTimerEvent * event)
{
    if (autoGeometry_) {
        QObject * geometryHelper = ::geometryHelper();
        QObject * geometry = nullptr;
        if (geometryHelper) {
            QPointF offset = ink_->mapToParent({0, 0});
            QMetaObject::invokeMethod(geometryHelper, "approxGeometryFinish",Q_RETURN_ARG(QObject*,geometry),
                                      Q_ARG(void*,autoGeometry_),
                                      Q_ARG(QPointF,offset));
        }
        if (geometry) {
            geometry->setProperty("strokes", QVariant::fromValue(autoStrokes_));
            Control::fromItem(ink_)->resource()->page()->addResource(qobject_cast<ResourceView*>(geometry));
            ink_->Strokes()->Remove(autoStrokes_);
        }
        autoStrokes_.reset();
        autoGeometry_ = nullptr;
    }
    killTimer(event->timerId());
}
