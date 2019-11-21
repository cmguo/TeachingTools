#ifndef WHITINGGRID_H
#define WHITINGGRID_H

#include <QGraphicsItem>
#include "TeachingTools_global.h"

enum WhitingGridType
{
    TinWordFormat, // 田字格
    FourLinesAndThreeGrids, // 四线三格
    PinYinTinGrids // 拼音田字格
};
class TEACHINGTOOLS_EXPORT WhitingGrid : public QGraphicsItem
{
public:
    WhitingGrid(QGraphicsItem *parent = nullptr);
    WhitingGrid(int h,WhitingGridType type = WhitingGridType::TinWordFormat,QGraphicsItem *parent = nullptr);
    QRectF boundingRect()const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;

    void paintTinWordFormat(QPainter *painter,const QStyleOptionGraphicsItem *option, QWidget *widget);
    void paintFourLinesAndThreeGrids(QPainter *painter,const QStyleOptionGraphicsItem *option, QWidget *widget);
    void paintPinYinTinGrids(QPainter *painter,const QStyleOptionGraphicsItem *option, QWidget *widget);

    virtual bool sceneEventFilter(QGraphicsItem *watched, QEvent *event) override;
    virtual QVariant itemChange(GraphicsItemChange change, const QVariant &value) override;
    QGraphicsItem* createControlBar();

    void setDotLineColor(QColor &color);
    void setDotLineWidth(int width);

    void setRealLineWidth(int width);

    void setRealLineColor(QColor &color);

    int getDotLineWidth() const;

    QColor getDotLineColor() const;

    int getRealLineWidth() const;

    QColor getRealLineColor() const;

   WhitingGridType getType();

   void setType(WhitingGridType type);

public:
   void addGrid();

   void decGrid();
private:
   void adjustWidth();
   void adjustControlItemPos();

private:
    QGraphicsPixmapItem *addItem;
    QGraphicsPixmapItem *decItem;
    QGraphicsRectItem *controlItem;
    qreal m_width;
    qreal m_height;
    QColor m_dotLineColor;
    int m_dotLineWidth;
    QColor m_realLineColor;
    int m_realLineWidth;
    WhitingGridType type_;
    int gridCount_ = 1;
    int padding = 1;
    float tinWidthHeihtRatio = 1.0f;
    float fourLineThreeGridsWidthHeihtRatio = 310.0f/152.0f;
    float pinYinTinWidthHeightRatio = 240.0f/369.0f;
    QSizeF newScaleSize;
    QSizeF controlItemSize=QSizeF(35,35);

};

#endif // WHITINGGRID_H
