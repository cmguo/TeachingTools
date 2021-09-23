#ifndef MINDMAP_H
#define MINDMAP_H

#include <core/resourceview.h>

class MindNode;
class MindViewTemplate;
class MindMapControl;

class MindMap : public ResourceView
{
    Q_OBJECT
public:
    Q_INVOKABLE MindMap(Resource * res);

public:
    void setContent(QByteArray const & data);

    QByteArray getContent() const;

private:
    friend class MindMapControl;
    MindNode * node_;
    MindViewTemplate * template_;
};

#endif // MINDMAP_H
