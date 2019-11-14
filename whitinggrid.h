#ifndef WHITINGGRID_H
#define WHITINGGRID_H

#include <QGraphicsItem>
#include "TeachingTools_global.h"

enum WhitingGridType
{
    TinWordFormat, // 田字格
    FourLinesAndThreeGrids // 四线三格
};
class TEACHINGTOOLS_EXPORT WhitingGrid : public QGraphicsObject
{
    Q_OBJECT
    Q_PROPERTY(int m_dotLineWidth READ getDotLineWidth WRITE setDotLineWidth)
    Q_PROPERTY(int m_realLineWidth READ getRealLineWidth WRITE setRealLineWidth)
    Q_PROPERTY(QColor m_realLineColor READ getRealLineColor WRITE setRealLineColor)
    Q_PROPERTY(QColor m_dotLineColor READ getDotLineColor WRITE setDotLineColor)
    Q_PROPERTY(WhitingGridType type_ READ getType WRITE setType)
public:
    WhitingGrid(QGraphicsObject *parent = nullptr);
    WhitingGrid(int w,int h,WhitingGridType type = WhitingGridType::TinWordFormat,QGraphicsObject *parent = nullptr);
    QRectF boundingRect()const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;

    void paintTinWordFormat(QPainter *painter,const QStyleOptionGraphicsItem *option, QWidget *widget);
    void paintFourLinesAndThreeGrids(QPainter *painter,const QStyleOptionGraphicsItem *option, QWidget *widget);

    void setDotLineColor(QColor &color){

        this->m_dotLineColor = color;
        this->update(boundingRect());
    }
    void setDotLineWidth(int width){
        this->m_dotLineWidth =  width;
        this->update(boundingRect());
    }

    void setRealLineWidth(int width){
        this->m_realLineWidth = width;
        this->update(boundingRect());
    }

    void setRealLineColor(QColor &color){
        this->m_realLineColor = color;
        this->update(boundingRect());
    }

    int getDotLineWidth() const {
        return this->m_dotLineWidth;
    }

    QColor getDotLineColor() const{
        return this->m_dotLineColor;
    }

    int getRealLineWidth() const{
        return this->m_realLineWidth;
    }

    QColor getRealLineColor() const{
        return this->m_realLineColor;
    }

   WhitingGridType getType(){
        return type_;
    }

   void setType(WhitingGridType type){
       this->type_ = type;
   }

public:


private:
    qreal m_width;
    qreal m_height;
    QColor m_dotLineColor;
    int m_dotLineWidth;
    QColor m_realLineColor;
    int m_realLineWidth;
    WhitingGridType type_;
};

#endif // WHITINGGRID_H
