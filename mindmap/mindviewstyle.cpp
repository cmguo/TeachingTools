#include "mindnode.h"
#include "mindviewstyle.h"

#include <QFontMetrics>
#include <QJsonArray>
#include <QJsonObject>
#include <QPainter>
#include <QPainterPath>

MindViewStyle::MindViewStyle()
{
     textColor_ = connectColor_ = QColor(Qt::black);
}

static QPointF readPoint(QJsonValue const & json, QPointF const & dflt = {})
{
    if (json.isArray()) {
        QJsonArray arr = json.toArray();
        qreal a0 = arr[0].toDouble();
        qreal a1 = arr[1].toDouble();
        return {a0, a1};
    } else if (json.isObject()) {
        QJsonObject obj = json.toObject();
        qreal a0 = obj.value("x").toDouble();
        qreal a1 = obj.value("y").toDouble();
        return {a0, a1};
    } else {
        return dflt;
    }
}

static QRectF readPadding(QJsonValue const & json)
{
    if (json.isDouble()) {
        qreal v = json.toDouble();
        return {-v, -v, v * 2, v * 2};
    } else if (json.isArray()) {
        QJsonArray arr = json.toArray();
        qreal a0 = arr[0].toDouble();
        qreal a1 = arr[1].toDouble();
        qreal a2 = arr[2].toDouble();
        qreal a3 = arr[3].toDouble();
        return {-a0, -a1, a0 + a2, a1 + a3};
    } else if (json.isObject()) {
        QJsonObject obj = json.toObject();
        qreal a0 = obj.value("left").toDouble();
        qreal a1 = obj.value("top").toDouble();
        qreal a2 = obj.value("right").toDouble();
        qreal a3 = obj.value("bottom").toDouble();
        return {-a0, -a1, a0 + a2, a1 + a3};
    } else {
        return {};
    }
}

static void readRadius(QJsonValue const & json, QRectF & up, QRectF & down)
{
    if (json.isDouble()) {
        qreal v = json.toDouble();
        up = QRectF(v, v, -v * 2, 0);
        down = QRectF(v, -v, -v * 2, 0);
    } else if (json.isArray()) {
        QJsonArray arr = json.toArray();
        qreal a0 = arr[0].toDouble();
        qreal a1 = arr[1].toDouble();
        qreal a2 = arr[2].toDouble();
        qreal a3 = arr[3].toDouble();
        qreal a4 = arr[4].toDouble();
        qreal a5 = arr[5].toDouble();
        qreal a6 = arr[6].toDouble();
        qreal a7 = arr[7].toDouble();
        up = QRectF(a0, a1, -a2 - a0, a3 - a1);
        down = QRectF(a4, -a5, -a6 - a4, -a7 + a5);
    }
}

MindViewStyle::MindViewStyle(const QJsonObject &json)
    : MindViewStyle()
{
    QJsonObject font = json.value("font").toObject();
    textFont_ = QFont(font.value("family").toString("Microsoft YaHei"),
                      font.value("size").toDouble(-1),
                      font.value("weight").toDouble(-1),
                      font.value("italic").toBool(false));
    textColor_ = QColor(json.value("textColor").toString());
    backgroundColor_ = QColor(json.value("backgroundColor").toString());
    borderColor_ = QColor(json.value("borderColor").toString());
    connectColor_ = QColor(json.value("connectColor").toString());
    outerPadding_ = readPadding(json.value("outerPadding"));
    borderWidth_ = readPadding(json.value("borderWidth"));
    innerPadding_ = readPadding(json.value("innerPadding"));
    readRadius(json.value("cornerRadius"), cornerRadiusUp_, cornerRadiusDown_);
    inPort_ = readPoint(json.value("inPort"), {0, 0.5});
    outPort_ = readPoint(json.value("outPort"), {1, 0.5});
    switchPort_ = readPoint(json.value("switchPort"), {1, 0.5});
}

QSizeF MindViewStyle::measureNode(MindNode *node) const
{
    QRectF rect = QFontMetrics(textFont_).boundingRect(node->title);
    rect.adjust(innerPadding_.left(), innerPadding_.top(), innerPadding_.right(), innerPadding_.bottom());
    rect.adjust(borderWidth_.left(), borderWidth_.top(), borderWidth_.right(), borderWidth_.bottom());
    rect.adjust(outerPadding_.left(), outerPadding_.top(), outerPadding_.right(), outerPadding_.bottom());
    return rect.size();
}

static QPointF normalizePort(QPointF port, QSizeF const & size)
{
    if (port.x() < 0)
        port.setX(size.width() + port.x());
    else if (port.x() <= 1)
        port.setX(size.width() * port.x());
    if (port.y() < 0)
        port.setY(size.height() + port.y());
    else if (port.y() <= 1)
        port.setY(size.height() * port.y());
    return port;
}

QPointF MindViewStyle::inPort(const QSizeF &size) const
{
    return normalizePort(inPort_, size);
}

QPointF MindViewStyle::outPort(const QSizeF &size) const
{
    return normalizePort(outPort_, size);
}

QPointF MindViewStyle::switchPort(const QSizeF &size) const
{
    return normalizePort(switchPort_, size);
}

QPainterPath MindViewStyle::roundedRect(QRectF const & rect) const
{
    QPainterPath path;
    path.moveTo(rect.topLeft() + QPointF{0, cornerRadiusUp_.top()});
    if (!cornerRadiusUp_.topLeft().isNull()) {
        QRectF corner{rect.topLeft(), rect.topLeft() + cornerRadiusUp_.topLeft() * 2};
        path.arcTo(corner, 180, -90);
    }
    path.lineTo(rect.topRight() + QPointF{cornerRadiusUp_.right(), 0});
    if (!cornerRadiusUp_.bottomRight().isNull()) {
        QRectF corner = QRectF{rect.topRight(), rect.topRight() + cornerRadiusUp_.bottomRight() * 2}.normalized();
        path.arcTo(corner, 90, -90);
    }
    path.lineTo(rect.bottomRight() + QPointF{0, cornerRadiusDown_.bottom()});
    if (!cornerRadiusUp_.bottomRight().isNull()) {
        QRectF corner = QRectF(rect.bottomRight(), rect.bottomRight() + cornerRadiusDown_.bottomRight() * 2).normalized();
        path.arcTo(corner, 0, -90);
    }
    path.lineTo(rect.bottomLeft() + QPointF{cornerRadiusDown_.left(), 0});
    if (!cornerRadiusUp_.topLeft().isNull()) {
        QRectF corner = QRectF{rect.bottomLeft(), rect.bottomLeft() + cornerRadiusDown_.topLeft() * 2}.normalized();
        path.arcTo(corner, 270, -90);
    }
    path.closeSubpath();
    return path;
}

void MindViewStyle::applyTo(QPainter *painter, QRectF & rect) const
{
    QRectF backgroundRect = rect.adjusted(-outerPadding_.left(), -outerPadding_.top(), -outerPadding_.right(), -outerPadding_.bottom());
    QRectF borderRect = backgroundRect.adjusted(-borderWidth_.left(), -borderWidth_.top(), -borderWidth_.right(), -borderWidth_.bottom());
    QRectF innerRect = borderRect.adjusted(-innerPadding_.left(), -innerPadding_.top(), -innerPadding_.right(), -innerPadding_.bottom());
    if (backgroundColor_.isValid() || borderColor_.isValid()) {
//        if (cornerRadiusUp_.topLeft().isNull() && cornerRadiusUp_.isNull()
//                && cornerRadiusDown_.topLeft().isNull() && cornerRadiusDown_.isNull()) {
//            painter->setPen(Qt::NoPen);
//            if (backgroundColor_.isValid()) {
//                painter->setBrush(backgroundColor_);
//                painter->drawRect(backgroundRect);
//            }
//            if (borderColor_.isValid()) {
//                backgroundRect -= borderRect;
//                QPainterPath borderPath = roundedRect(borderRect);
//                borderPath = backgroundPath - borderPath;
//                painter->setBrush(borderColor_);
//                painter->drawPath(borderPath);
//            }
//        } else {
        QPainterPath backgroundPath = roundedRect(backgroundRect);
        painter->setPen(Qt::NoPen);
        if (backgroundColor_.isValid()) {
            painter->setBrush(backgroundColor_);
            painter->drawPath(backgroundPath);
        }
        if (borderColor_.isValid()) {
            QPainterPath borderPath = roundedRect(borderRect);
            borderPath = backgroundPath.subtracted(borderPath).simplified();
            painter->setBrush(borderColor_);
            painter->drawPath(borderPath);
        }
//        }
    }
    painter->setFont(textFont_);
    painter->setPen(textColor_);
    painter->setBrush(textColor_);
    rect = innerRect;
}

QPointF MindViewStyle::textOffset() const
{
    QPointF off = {-outerPadding_.left(), -outerPadding_.top()};
    off += QPointF{-borderWidth_.left(), -borderWidth_.top()};
    off += QPointF{-innerPadding_.left(), -innerPadding_.top()};
    return off;
}
