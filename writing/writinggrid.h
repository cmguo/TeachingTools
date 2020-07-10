#ifndef WRITINGGRID_H
#define WRITINGGRID_H

#include "TeachingTools_global.h"

#include <InkCanvas_global.h>

#include <QGraphicsObject>

INKCANVAS_FORWARD_DECLARE_CLASS(InkCanvas);
INKCANVAS_FORWARD_DECLARE_CLASS(StrokeCollection);

enum WritingGridType
{
    TinWordFormat, // 田字格
    PinYin, //拼音格
    PinYinTinGrids, // 拼音田字格
    FourLinesAndThreeGrids // 四线三格
};
class TEACHINGTOOLS_EXPORT WritingGrid : public QGraphicsObject
{
    Q_OBJECT

    Q_PROPERTY(QSharedPointer<QtInkCanvas::StrokeCollection> strokes READ strokes WRITE setStrokes)
    Q_PROPERTY(int gridCount READ gridCount WRITE setGridCount)

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

   InkCanvas * inkCanvas()
   {
       return ink;
   }

   QSharedPointer<StrokeCollection> strokes();

   void setStrokes(QSharedPointer<StrokeCollection> strokes);

   int gridCount() const;

   void setGridCount(int n);

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
    QGraphicsPixmapItem *inkItem;
	QGraphicsPixmapItem *inkEraseItem;
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
    QSizeF controlItemSize=QSizeF(72,72);
    QSize itemSize=QSize(40,40);
    InkCanvas * ink ;
    float m_adapterRatio = 1.0f;

};

#endif // WRITINGGRID_H
