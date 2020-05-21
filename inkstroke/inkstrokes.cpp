#include "inkstrokerenderer.h"
#include "inkstrokes.h"

#include <core/resource.h>

#include <Windows/Ink/strokecollection.h>
#include <Windows/Ink/stroke.h>
#include <Windows/Input/styluspointcollection.h>
#include <stroke/strokesrenderer.h>

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
        strokes_->setParent(o.strokes().get());
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

QtPromise::QPromise<void> InkStrokes::load(QSizeF const & maxSize, QSharedPointer<DrawingAttributes> attr)
{
    if (strokes_ != nullptr)
        return QtPromise::QPromise<void>::resolve();
    return Strokes::load().then([this, l = life(), attr, maxSize](StrokesReader * reader) {
        if (l.isNull()) {
            delete reader;
            throw std::runtime_error("dead");
        }
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
