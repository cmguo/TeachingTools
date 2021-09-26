#include "arcconnector.h"
#include "bezierconnector.h"
#include "mindviewstyle.h"
#include "mindviewtemplate.h"
#include "simpleconnector.h"
#include "simpleswitch.h"
#include "simpleview.h"

#include <QJsonArray>
#include <QJsonObject>

MindViewTemplate::MindViewTemplate()
{
    levelPadding = 60;
    siblinPadding = 30;
    viewStyles.append(new MindViewStyle());
}

MindViewTemplate::MindViewTemplate(const QJsonObject &json)
{
    levelPadding = json.value("levelPadding").toDouble(60);
    siblinPadding = json.value("siblinPadding").toDouble(30);
    QJsonArray vss = json.value("viewStyles").toArray();
    for (auto vs : vss) {
        viewStyles.append(new MindViewStyle(vs.toObject()));
    }
    QJsonArray cts = json.value("connectorTypes").toArray();
    for (auto ct : cts) {
        connectorTypes.append(ct.toString().toUtf8());
    }
    QJsonArray cws = json.value("connectorWidths").toArray();
    for (auto cw : cws) {
        connectorWidths.append(cw.toDouble());
    }
    defaultNode = MindNode(json.value("defaultNode").toObject());
}

MindViewTemplate::~MindViewTemplate()
{
    for (auto s : viewStyles)
        delete s;
}

void MindViewTemplate::push(QPointF & pos)
{
    if (++level == 1) {
        xoffset = 0;
        yoffset = 0;
        xmax = 0;
        ymax = 0;
    }
    pos = {xoffset, yoffset};
}

void MindViewTemplate::pop(QPointF const & pos)
{
    if (xoffset > xmax)
        xmax = xoffset;
    if (yoffset > ymax)
        ymax = yoffset;
    xoffset = pos.x();
    --level;
}

MindNode MindViewTemplate::createNode()
{
    return defaultNode;
}

MindNodeView *MindViewTemplate::createView(MindNode *node)
{
    MindViewStyle const * style = level < viewStyles.size() ? viewStyles.at(level) : viewStyles.back();
    MindNodeView * view = new SimpleView(node);
    view->setViewStyle(style);
    return view;
}

MindSwitch *MindViewTemplate::createSwitch()
{
    return new SimpleSwitch;
}

MindConnector *MindViewTemplate::createConnector()
{
    //QByteArray type = level < connectorTypes.size() ? connectorTypes.at(level) : connectorTypes.back();
    return new BezierConnector;
}
