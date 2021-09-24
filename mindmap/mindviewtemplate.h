#ifndef MINDVIEWTEMPLATE_H
#define MINDVIEWTEMPLATE_H

#include "mindnode.h"

#include <QColor>
#include <QFont>
#include <QRectF>
#include <QVector>

class MindNode;
class MindNodeView;
class MindConnector;
class MindViewStyle;

class QJsonObject;

class MindViewTemplate
{
public:
    MindViewTemplate();

    MindViewTemplate(QJsonObject const & json);

    ~MindViewTemplate();

public:
    qreal levelPadding;
    qreal siblinPadding;

    QVector<MindViewStyle*> viewStyles;
    QVector<QByteArray> connectorTypes;
    QVector<qreal> connectorWidths;

    MindNode defaultNode;

public:
    // state
    int level = 0;
    qreal xoffset = 0;
    qreal yoffset = 0;
    qreal xmax = 0;

public:
    void push(QPointF & pos);

    void pop(QPointF const & pos);

    MindNode createNode();

    MindNodeView * createView(MindNode * node);

    MindConnector * createConnector();
};

#endif // MINDVIEWTEMPLATE_H
