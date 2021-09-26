#ifndef MINDSWITCH_H
#define MINDSWITCH_H

#include "mindbaseview.h"

#include <QPointF>

class MindNodeView;

class MindSwitch : public MindBaseView
{
public:
    virtual ~MindSwitch() {}

public:
    void setHover(bool hover);

protected:
    QPointF pos() const;

    bool expanded() const;

    bool hover_ = false;
};

#endif // MINDSWITCH_H
