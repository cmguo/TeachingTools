#include "inkstrokerenderer.h"

#include <Windows/Ink/stroke.h>

InkStrokeRenderer::InkStrokeRenderer(StrokeReader* reader, QSizeF const & maxSize, QSharedPointer<StrokeCollection> strokes,
                                     QSharedPointer<DrawingAttributes> attr, QObject *parent)
    : StrokeRenderer(reader, parent)
    , strokes_(strokes)
    , da_(attr)
    , stylusPoints_(new StylusPointCollection)
    , destSize_(maxSize)
{
}

QSizeF InkStrokeRenderer::size() const
{
    return QSizeF(pointRange_[0], pointRange_[1]) * scale_;
}

void InkStrokeRenderer::setMaximun(const StrokePoint &max)
{
    pointRange_ = max;
    scale_ = qMin(destSize_.width() / max[0], destSize_.height() / max[1]);
    pressureScale_ = static_cast<float>(scale_ / max[2]);
}

void InkStrokeRenderer::addPoint(const StrokePoint &pt)
{
    StylusPoint point(pt[0] * scale_, pt[1] * scale_, pt[2] * pressureScale_);
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
