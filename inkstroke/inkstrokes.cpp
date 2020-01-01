#include "inkstrokes.h"

#include <Windows/Ink/strokecollection.h>
#include <Windows/Ink/stroke.h>
#include <Windows/Input/styluspointcollection.h>

InkStrokes::InkStrokes(Resource *res)
    : Strokes(res, Splittable)
    , next_(nullptr)
    , prev_(nullptr)
{
    if (url().path().isEmpty()) {
        strokes_.reset(new StrokeCollection);
    }
}

InkStrokes::InkStrokes(InkStrokes &o)
    : Strokes(o)
    , next_(o.next_)
    , prev_(&o)
{
    strokes_.reset(new StrokeCollection);
    strokes_.swap(o.strokes_);
    if (o.next_)
        o.next_->prev_ = this;
    o.next_ = this;
    flags_.setFlag(StickOn);
    flags_.setFlag(CanDelete);
    flags_.setFlag(Splittable, false);
    flags_.setFlag(TopMost, false);
    flags_.setFlag(CanCopy, false);
}

InkStrokes::~InkStrokes()
{
    if (prev_)
        prev_->next_ = next_;
    if (next_)
        next_->prev_ = prev_;
}

QSharedPointer<StrokeCollection> InkStrokes::strokes()
{
    return strokes_;
}

QtPromise::QPromise<void> InkStrokes::load(QSizeF const & size, QSharedPointer<DrawingAttributes> attr)
{
    if (strokes_ != nullptr)
        return QtPromise::QPromise<void>::resolve();
    strokes_.reset(new StrokeCollection);
    return Strokes::load().then([this, l = life(), size, attr]() {
        //if (l.isNull())
        //    return;
        if (points().empty())
            return;
        QSharedPointer<StylusPointCollection> stylusPoints(new StylusPointCollection);
        //render debug feedback?
        //QUuid guid("52053C24-CBDD-4547-AAA1-DEFEBF7FD1E1");
        for (auto & pt : points()) {
            if (pt[2] < 0) {
                QSharedPointer<Stroke> stroke(new Stroke(stylusPoints, attr));
                //stroke->AddPropertyData(guid, 4.0);
                strokes_->AddItem(stroke);
                stylusPoints.reset(new StylusPointCollection);
            } else {
                StylusPoint point((1 + static_cast<double>(pt[0])) / 2 * size.width(),
                        (1 - static_cast<double>(pt[1])) * size.height(), pt[2]);
                stylusPoints->AddItem(point);
            }
        }
        if (stylusPoints->size() > 0) {
            QSharedPointer<Stroke> stroke(new Stroke(stylusPoints, attr));
            strokes_->AddItem(stroke);
        }
    });
}

InkStrokes *InkStrokes::clone() const
{
    if (strokes_->empty())
        return nullptr;
    InkStrokes* ink = new InkStrokes(const_cast<InkStrokes&>(*this));
    emit cloned();
    return ink;
}
