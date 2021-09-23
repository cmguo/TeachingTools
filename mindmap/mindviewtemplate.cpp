#include "arcconnector.h"
#include "bezierconnector.h"
#include "mindviewtemplate.h"
#include "simpleconnector.h"
#include "simpleview.h"

#include <QJsonArray>
#include <QJsonObject>

MindViewTemplate::MindViewTemplate()
{
    levelPadding = 60;
    siblinPadding = 30;
}

MindViewTemplate::MindViewTemplate(const QJsonObject &json)
{
    levelPadding = json.value("levelPadding").toDouble();
    siblinPadding = json.value("siblinPadding").toDouble();
    QJsonArray lts = json.value("lineTypes").toArray();
    for (auto lt : lts) {
        lineTypes.append(lt.toInt());
    }
    QJsonArray lws = json.value("lineWidths").toArray();
    for (auto lw : lws) {
        lineWidths.append(lw.toDouble());
    }
    QJsonArray vts = json.value("viewTypes").toArray();
    for (auto vt : vts) {
        viewTypes.append(vt.toString().toUtf8());
    }
    QJsonArray cts = json.value("connectorTypes").toArray();
    for (auto ct : vts) {
        connectorTypes.append(ct.toString().toUtf8());
    }
}

void MindViewTemplate::push(QPointF & pos)
{
    if (++level == 0) {
        xoffset = 0;
        yoffset = 0;
        xmax = 0;
    }
    pos = {xoffset, yoffset};
}

void MindViewTemplate::pop(QPointF const & pos)
{
    if (xoffset > xmax)
        xmax = xoffset;
    xoffset = pos.x();
    --level;
}

int MindViewTemplate::lineType() const
{
    return level < lineTypes.size() ? lineTypes.at(level) : lineTypes.back();
}

qreal MindViewTemplate::lineWidth() const
{
    return level < lineWidths.size() ? lineWidths.at(level) : lineWidths.back();
}

MindNodeView *MindViewTemplate::createView(MindNode *node)
{
    //QByteArray type = level < viewTypes.size() ? viewTypes.at(level) : viewTypes.back();
    return new SimpleView(node);
}

MindConnector *MindViewTemplate::createConnector()
{
    //QByteArray type = level < connectorTypes.size() ? connectorTypes.at(level) : connectorTypes.back();
    return new BezierConnector;
}
