#include "inkstrokes.h"

#include <core/resource.h>

#include <Windows/Ink/strokecollection.h>
#include <Windows/Ink/stroke.h>
#include <Windows/Input/styluspointcollection.h>
#include <stroke/strokerenderer.h>

InkStrokes::InkStrokes(Resource *res)
    : Strokes(res)
    , renderer_(nullptr)
    , next_(nullptr)
    , prev_(nullptr)
{
    if (url().scheme() == res->type()) {
        flags_.setFlag(TopMost);
        flags_.setFlag(Splittable);
        flags_.setFlag(CanCopy, false);
        flags_.setFlag(CanDelete, false);
        strokes_.reset(new StrokeCollection);
    }
}

InkStrokes::InkStrokes(InkStrokes &o)
    : Strokes(o)
    , renderer_(nullptr)
    , next_(o.next_)
    , prev_(&o)
{
    if (o.flags().testFlag(Splittable)) {
        strokes_.reset(new StrokeCollection);
        strokes_.swap(o.strokes_);
        if (o.next_)
            o.next_->prev_ = this;
        o.next_ = this;
        //flags_.setFlag(StickOn);
        flags_.setFlag(CanDelete);
        flags_.setFlag(Splittable, false);
        flags_.setFlag(TopMost, false);
    } else {
        strokes_ = o.strokes_->Clone();
    }
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

class InkStrokeRenderer : public StrokeRenderer
{
public:
    InkStrokeRenderer(StrokeReader* reader, QSizeF const & maxSize, QSharedPointer<StrokeCollection> strokes, QSharedPointer<DrawingAttributes> attr, QObject * parent)
        : StrokeRenderer(reader, parent)
        , strokes_(strokes)
        , da_(attr)
        , stylusPoints_(new StylusPointCollection)
        , destSize_(maxSize)
    {
    }

public:
    QSizeF size() const
    {
        return QSizeF(pointRange_[0], pointRange_[1]) * scale_;
    }

    // IDynamicRenderer interface
protected:
    virtual void setMaximun(const StrokePoint &max) override
    {
        pointRange_ = max;
        scale_ = qMin(destSize_.width() / max[0], destSize_.height() / max[1]);
        pressureScale_ = static_cast<float>(scale_ / max[2]);
    }

    virtual void addPoint(const StrokePoint &pt) override
    {
        StylusPoint point(pt[0] * scale_, pt[1] * scale_, pt[2] * pressureScale_);
        stylusPoints_->AddItem(point);
    }

    virtual void endStroke() override
    {
        QSharedPointer<Stroke> stroke(new Stroke(stylusPoints_, da_));
        //stroke->AddPropertyData(guid, 4.0);
        strokes_->AddItem(stroke);
        stylusPoints_.reset(new StylusPointCollection);
    }

    virtual void startDynamic() override
    {
    }

    virtual void finish() override
    {
        if (stylusPoints_->size())
            endStroke();
    }

private:
    QSharedPointer<StrokeCollection> strokes_;
    QSharedPointer<DrawingAttributes> da_;
    QSharedPointer<StylusPointCollection> stylusPoints_;
    StrokePoint pointRange_;
    QSizeF destSize_;
    qreal scale_;
    float pressureScale_;
};

QtPromise::QPromise<void> InkStrokes::load(QSizeF const & maxSize, QSharedPointer<DrawingAttributes> attr)
{
    if (strokes_ != nullptr)
        return QtPromise::QPromise<void>::resolve();
    return Strokes::load().then([this, l = life(), attr, maxSize](StrokeReader * reader) {
        //if (l.isNull())
        //    throw std::runtime_error("dead");
        strokes_.reset(new StrokeCollection);
        renderer_ = new InkStrokeRenderer(reader, maxSize, strokes_, attr, this);
        renderer_->start();
    });
}

QSizeF InkStrokes::size() const
{
    return renderer_->size();
}

void InkStrokes::clear()
{
    strokes_->ClearItems();
    if (next_)
        next_->clear();
}

InkStrokes *InkStrokes::clone() const
{
    if (flags_.testFlag(Splittable)) {
        if (strokes_->empty())
            return nullptr;
        InkStrokes* ink = new InkStrokes(const_cast<InkStrokes&>(*this));
        emit cloned();
        return ink;
    } else {
        return new InkStrokes(const_cast<InkStrokes&>(*this));
    }
}
