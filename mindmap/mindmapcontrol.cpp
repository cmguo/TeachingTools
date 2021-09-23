#include "mindmapcontrol.h"
#include "mindmap.h"
#include "mindmapitem.h"

MindMapControl::MindMapControl(ResourceView *res)
    : Control(res, {}, {CanScale, CanRotate, CanSelect})
{
}

ControlView *MindMapControl::create(ControlView *parent)
{
    (void) parent;
    MindMapItem * item = new MindMapItem;
    if (flags_.testFlag(RestoreSession)) {
        MindMap * map = qobject_cast<MindMap*>(res_);
        item->load(map->template_, map->node_);
    }
    return item;
}

void MindMapControl::attached()
{
    item_->setFocus();
    if (flags_.testFlag(RestoreSession)) {
        loadFinished();
        return;
    }
    loadData();
}

void MindMapControl::onData(QByteArray data)
{
    MindMap * map = qobject_cast<MindMap*>(res_);
    map->setContent(data);
    static_cast<MindMapItem*>(item_)->load(map->template_, map->node_);
    loadFinished();
}
