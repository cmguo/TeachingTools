#include "inkstrokerenderer.h"

#include <Windows/Ink/stroke.h>

INKCANVAS_USE_NAMESPACE

InkStrokeRenderer::InkStrokeRenderer(StrokesReader* reader, QSizeF const & maxSize, QSharedPointer<StrokeCollection> strokes,
                                     QSharedPointer<DrawingAttributes> attr, QObject *parent)
    : StrokesRenderer(reader, parent)
    , strokes_(strokes)
    , da_(attr)
    , destSize_(maxSize)
{
}

QSizeF InkStrokeRenderer::size() const
{
    return QSizeF(maximun_.x, maximun_.y) * scale_;
}

void InkStrokeRenderer::setMaximun(const StrokePoint &max)
{
    scale_ = qMin(destSize_.width() / max.x, destSize_.height() / max.y);
    pressureScale_ = static_cast<float>(1.0 / max.p);
}

void InkStrokeRenderer::startStroke(const StrokePoint &pt)
{
    stylusPoints_.reset(new StylusPointCollection);
    StylusPoint point(pt.x * scale_, pt.y * scale_, pt.p * pressureScale_);
    stylusPoints_->Add(point);
}

void InkStrokeRenderer::addPoint(const StrokePoint &pt)
{
    StylusPoint point(pt.x * scale_, pt.y * scale_, pt.p * pressureScale_);
    stylusPoints_->Add(point);
}

void InkStrokeRenderer::endStroke()
{
    QSharedPointer<Stroke> stroke(new Stroke(stylusPoints_, da_));
    //stroke->AddPropertyData(guid, 4.0);
    strokes_->Add(stroke);
    stylusPoints_.reset();
}
