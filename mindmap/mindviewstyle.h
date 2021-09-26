#ifndef MINDVIEWSTYLE_H
#define MINDVIEWSTYLE_H

#include <QColor>
#include <QFont>
#include <QRectF>
#include <QSizeF>

class MindNode;
class QPainter;

class MindViewStyle
{
public:
    MindViewStyle();

    MindViewStyle(QJsonObject const & json);

public:
    QSizeF measureNode(MindNode * node) const;

    QPointF inPort(QSizeF const & size) const;

    QPointF outPort(QSizeF const & size) const;

    QPointF switchPort(QSizeF const & size) const;

    void applyTo(QPainter * painter, QRectF & rect) const;

    QFont textFont() const { return textFont_; }

    QColor textColor() const { return textColor_; }

    QColor connectColor() const { return connectColor_; }

private:
    QPainterPath roundedRect(QRectF const & rect) const;

private:
    QFont textFont_;
    QColor textColor_;
    QColor backgroundColor_;
    QColor borderColor_;
    QColor connectColor_;
    QRectF borderWidth_;
    QRectF cornerRadiusUp_;
    QRectF cornerRadiusDown_;
    QRectF outerPadding_;
    QRectF innerPadding_;
    QPointF inPort_;
    QPointF outPort_;
    QPointF switchPort_;
};

#endif // MINDVIEWSTYLE_H
