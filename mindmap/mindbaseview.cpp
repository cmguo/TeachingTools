#include "mindbaseview.h"
#include "mindnodeview.h"

void MindBaseView::setParent(MindNodeView *parent)
{
    parent_ = parent;
}

bool MindBaseView::hasParent(MindNodeView * parent) const
{
    MindNodeView * p = parent_;
    while (p && p != parent) {
        p = p->parent_;
    }
    return p == parent;
}
