#ifndef WRITINGGRID_H
#define WRITINGGRID_H

#include <QGraphicsItem>
#include <Windows/Controls/inkcanvas.h>
#include "TeachingTools_global.h"

class InkCanvasStrokeCollectedEventArgs;

enum WritingGridType
{
    TinWordFormat, // 田字格
    FourLinesAndThreeGrids, // 四线三格
    PinYinTinGrids // 拼音田字格
};
class TEACHINGTOOLS_EXPORT WritingGrid : public QGraphicsItem
{
public:
    WritingGrid(QGraphicsItem *parent = nullptr);
    WritingGrid(int h,WritingGridType type = WritingGridType::TinWordFormat,QGraphicsItem *parent = nullptr);
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

   WritingGridType getType();

   void setType(WritingGridType type);

public:
   void addGrid();

   void decGrid();
private:
   void adjustWidth();
   void adjustControlItemPos();
   void adjustInkCanvas();
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
    WritingGridType type_;
    int gridCount_ = 1;
    int padding = 1;
    float tinWidthHeihtRatio = 1.0f;
    float fourLineThreeGridsWidthHeihtRatio = 310.0f/152.0f;
    float pinYinTinWidthHeightRatio = 240.0f/369.0f;
    QSizeF newScaleSize;
    QSizeF controlItemSize=QSizeF(35,35);
    InkCanvas * ink ;

};

#endif // WRITINGGRID_H
