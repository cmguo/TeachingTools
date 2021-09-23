#ifndef MINDVIEWTEMPLATE_H
#define MINDVIEWTEMPLATE_H

#include <QVector>

class MindNode;
class MindNodeView;
class MindConnector;

class QJsonObject;

class MindViewTemplate
{
public:
    MindViewTemplate();

    MindViewTemplate(QJsonObject const & json);

public:
    qreal levelPadding;
    qreal siblinPadding;

    QVector<int> lineTypes;
    QVector<qreal> lineWidths;
    QVector<QByteArray> viewTypes;
    QVector<QByteArray> connectorTypes;

public:
    // state
    int level = -1;
    qreal xoffset = 0;
    qreal yoffset = 0;
    qreal xmax = 0;

public:
    void push(QPointF & pos);

    void pop(QPointF const & pos);

    int lineType() const;

    qreal lineWidth() const;

    MindNodeView * createView(MindNode * node);

    MindConnector * createConnector();
};

#endif // MINDVIEWTEMPLATE_H
