#include "inkstrokes.h"

#include <Windows/Ink/strokecollection.h>
#include <Windows/Ink/stroke.h>
#include <Windows/Input/styluspointcollection.h>

InkStrokes::InkStrokes(Resource *res)
    : Strokes(res)
{
}

InkStrokes::InkStrokes(const InkStrokes &o)
    : Strokes(o)
{
}

QSharedPointer<StrokeCollection> InkStrokes::strokes()
{
    return strokes_;
}

QtPromise::QPromise<void> InkStrokes::load(QSizeF const & size, QSharedPointer<DrawingAttributes> attr)
{
    if (strokes_ != nullptr)
        return QtPromise::QPromise<void>::resolve();
    return Strokes::load().then([this, l = life(), size, attr]() {
        //if (l.isNull())
        //    return;
        if (points().empty())
            return;
        QSharedPointer<StylusPointCollection> stylusPoints(new StylusPointCollection);
        strokes_.reset(new StrokeCollection);
        //render debug feedback?
        //QUuid guid("52053C24-CBDD-4547-AAA1-DEFEBF7FD1E1");
        for (auto & pt : points()) {
            if (pt[2] < 0) {
                QSharedPointer<Stroke> stroke(new Stroke(stylusPoints, attr));
                //stroke->AddPropertyData(guid, 4.0);
                strokes_->AddItem(stroke);
                stylusPoints.reset(new StylusPointCollection);
            } else {
                StylusPoint point((1 + pt[0]) / 2 * size.width(), (1 - pt[1]) * size.height(), pt[2]);
                stylusPoints->AddItem(point);
            }
        }
        if (stylusPoints->size() > 0) {
            QSharedPointer<Stroke> stroke(new Stroke(stylusPoints, attr));
            strokes_->AddItem(stroke);
        }
    });
}
