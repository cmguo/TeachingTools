#ifndef MINDNODE_H
#define MINDNODE_H

#include <QList>
#include <QString>

class QJsonObject;

class MindNode
{
public:
    MindNode();

    MindNode(QJsonObject const & json);

public:
    QByteArray icon;
    QString title;

public:
    bool expanded() const { return expanded_; }

private:
    friend class MindNodeView;
    QList<MindNode> children_;
    bool expanded_ = false;
};

#endif // MINDNODE_H
