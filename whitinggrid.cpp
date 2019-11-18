#include "whitinggrid.h"

#include <QPainter>

WhitingGrid::WhitingGrid(QGraphicsObject *parent)
{
    WhitingGrid(100,100,WhitingGridType::TinWordFormat,parent);
}

WhitingGrid::WhitingGrid(int w,int h,WhitingGridType type,QGraphicsObject * parent):m_width(w),m_height(h),type_(type),QGraphicsObject(parent)
{
  m_realLineColor = Qt::red;
  m_dotLineColor = Qt::red;
  m_dotLineWidth = 1;
  m_realLineWidth = 2;
  setFlag(QGraphicsItem::ItemIsFocusable, true);
}

QRectF WhitingGrid::boundingRect() const
{
   return QRectF(0,0,m_width*gridCount_+2*padding,m_height+2*padding);
}

void WhitingGrid::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
        Q_UNUSED(option)
        Q_UNUSED(widget)
    switch (getType()) {
    case WhitingGridType::TinWordFormat:
        paintTinWordFormat(painter,option,widget);
        break;
     case WhitingGridType::FourLinesAndThreeGrids:
        paintFourLinesAndThreeGrids(painter,option,widget);
        break;
    }
}

void WhitingGrid::paintTinWordFormat(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
        QPen p = QPen(m_realLineColor,m_realLineWidth);
        painter->setPen(p);
        QRectF rect = boundingRect().adjusted(padding,padding,-padding,-padding);
        painter->setBrush(Qt::white);
        painter->drawRect(rect);
        for(int i = 1; i<gridCount_;i++){
           painter->drawLine(rect.x()+m_width*i,rect.y(),rect.x()+m_width*i,rect.bottom());
        }
        p.setColor(m_dotLineColor);
        p.setWidth(m_dotLineWidth);
        p.setStyle(Qt::DotLine);
        painter->setPen(p);
        painter->drawLine(rect.x(),rect.y()+rect.height()/2,rect.right(),rect.y()+rect.height()/2);
        for(int i = 0; i<gridCount_;i++){
           painter->drawLine(rect.x()+m_width/2+m_width*i,rect.y(),rect.x()+m_width/2+m_width*i,rect.bottom());
        }

}

void WhitingGrid::paintFourLinesAndThreeGrids(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
        QRectF rect = boundingRect().adjusted(padding,padding,-padding,-padding);
        QPen p = QPen(Qt::white,m_realLineWidth);
        painter->setPen(p);
        painter->setBrush(Qt::white);
        painter->drawRect(rect);
        p.setColor(m_realLineColor);
        painter->setPen(p);
        painter->drawLine(rect.x(),rect.y(),rect.right(),rect.y());
        painter->drawLine(rect.x(),rect.y()+rect.height()/3,rect.right(),rect.y()+rect.height()/3);
        painter->drawLine(rect.x(),rect.y()+rect.height()*2/3,rect.right(),rect.y()+rect.height()*2/3);
        painter->drawLine(rect.x(),rect.bottom(),rect.right(),rect.bottom());
}
