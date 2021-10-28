#include "loupetool.h"
#include "loupeitem.h"

#include <core/resourceview.h>
#include <core/resourcetransform.h>

LoupeTool::LoupeTool(ResourceView *res)
    : Control(res)
{
}

ControlView *LoupeTool::create(ControlView *parent)
{
    (void) parent;
    return new LoupeItem;
}

void LoupeTool::attached()
{
    connect(res_->transform2(), &ResourceTransform::changed, static_cast<LoupeItem*>(item_), &LoupeItem::transformChanged);
    loadFinished();
}

Control::SelectMode LoupeTool::selectTest(ControlView *child, ControlView *, const QPointF &, bool)
{
    return child ? Select : PassSelect;
}
