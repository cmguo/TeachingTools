#include "mindnode.h"

#include <QJsonArray>
#include <QJsonObject>

MindNode::MindNode()
{

}

MindNode::MindNode(QJsonObject const & json)
{
    title = json.value("title").toString();
    QJsonArray children = json.value("children").toArray();
    for (auto c : children) {
        children_.append(MindNode(c.toObject()));
    }
}
