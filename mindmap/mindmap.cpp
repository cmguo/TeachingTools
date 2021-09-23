#include "mindmap.h"
#include "mindnode.h"
#include "mindviewtemplate.h"

#include <QJsonDocument>
#include <QJsonObject>

MindMap::MindMap(Resource *res)
    : ResourceView(res, {}, {CanCopy})
{
}

void MindMap::setContent(const QByteArray &data)
{
    QJsonDocument doc = QJsonDocument::fromJson(data);
    QJsonObject obj = doc.object();
    QJsonValue tl = obj.value("template");
    if (tl.isString()) {
        template_ = new MindViewTemplate;
    } else {
        template_ = new MindViewTemplate(tl.toObject());
    }
    node_ = new MindNode(obj.value("content").toObject());
}

QByteArray MindMap::getContent() const
{
    return nullptr;
}
