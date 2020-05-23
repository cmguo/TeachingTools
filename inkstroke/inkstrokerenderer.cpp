#include "inkstrokerenderer.h"

#include <Windows/Ink/stroke.h>

InkStrokeRenderer::InkStrokeRenderer(StrokesReader* reader, QSizeF const & maxSize, QSharedPointer<StrokeCollection> strokes,
                                     QSharedPointer<DrawingAttributes> attr, QObject *parent)
    : StrokesRenderer(reader, parent)
    , strokes_(strokes)
    , da_(attr)
    , stylusPoints_(new StylusPointCollection)
    , destSize_(maxSize)
{
}

QSizeF InkStrokeRenderer::size() const
{
    return QSizeF(pointRange_.x, pointRange_.y) * scale_;
}

void InkStrokeRenderer::setMaximun(const StrokePoint &max)
{
    pointRange_ = max;
    scale_ = qMin(destSize_.width() / max.x, destSize_.height() / max.y);
    pressureScale_ = static_cast<float>(1.0 / max.p);
}

void InkStrokeRenderer::addPoint(const StrokePoint &pt)
{
    StylusPoint point(pt.x * scale_, pt.y * scale_, pt.p * pressureScale_);
    stylusPoints_->AddItem(point);
}

void InkStrokeRenderer::endStroke()
{
    QSharedPointer<Stroke> stroke(new Stroke(stylusPoints_, da_));
    //stroke->AddPropertyData(guid, 4.0);
    strokes_->AddItem(stroke);
    stylusPoints_.reset(new StylusPointCollection);
}

void InkStrokeRenderer::startDynamic()
{
}

void InkStrokeRenderer::finish()
{
    if (stylusPoints_->size())
        endStroke();
}
