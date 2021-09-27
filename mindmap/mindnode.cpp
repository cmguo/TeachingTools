#include "mindnode.h"

#include <QJsonArray>
#include <QJsonObject>

MindNode::MindNode()
{
    title = "请输入内容";
}

MindNode::MindNode(QJsonObject const & json)
{
    title = json.value("title").toString();
    QJsonArray children = json.value("children").toArray();
    for (auto c : children) {
        children_.append(MindNode(c.toObject()));
    }
}
