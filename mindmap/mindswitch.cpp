#include "mindswitch.h"
#include "mindnodeview.h"
#include "mindnode.h"

void MindSwitch::setHover(bool hover)
{
    hover_ = hover;
}

QPointF MindSwitch::pos() const
{
    return parent_->switchPort();
}

bool MindSwitch::empty() const
{
    return parent_->node()->children().empty();
}

bool MindSwitch::expanded() const
{
    return parent_->node()->expanded();
}
