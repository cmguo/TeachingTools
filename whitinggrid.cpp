#include "whitinggrid.h"

#include <QPainter>

WhitingGrid::WhitingGrid(QGraphicsObject *parent)
{
    WhitingGrid(300,WhitingGridType::TinWordFormat,parent);
}

WhitingGrid::WhitingGrid(int h,WhitingGridType type,QGraphicsObject * parent):m_height(h),type_(type),QGraphicsObject(parent)
{
  m_realLineColor = QColor(0xCACACA);
  m_dotLineColor = QColor(0xCACACA);
  m_dotLineWidth = 2;
  m_realLineWidth = 3;
  adjustWidth();
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
    case WhitingGridType::PinYinTinGrids:
        paintPinYinTinGrids(painter,option,widget);
        break;
    }
}

void WhitingGrid::paintTinWordFormat(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
        QPen p = QPen(m_realLineColor,m_realLineWidth);
        p.setJoinStyle(Qt::PenJoinStyle::MiterJoin);
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
        p.setJoinStyle(Qt::PenJoinStyle::MiterJoin);
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

void WhitingGrid::paintPinYinTinGrids(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{ // 绘制拼音田字格
    QPen p = QPen(m_realLineColor,m_realLineWidth);
    p.setJoinStyle(Qt::PenJoinStyle::MiterJoin);
    painter->setPen(p);
    QRectF rect = boundingRect().adjusted(padding,padding,-padding,-padding);
    painter->setBrush(Qt::white);
    painter->drawRect(rect);
    // 绘制三条实线
    painter->drawLine(rect.x(),rect.y()+rect.height()*44.5/369.0f,rect.right(),rect.y()+rect.height()*44.5/369.0f);
    painter->drawLine(rect.x(),rect.y()+rect.height()*87.5/369.0f,rect.right(),rect.y()+rect.height()*87.5/369.0f);
    painter->drawLine(rect.x(),rect.y()+rect.height()*130.5/369.0f,rect.right(),rect.y()+rect.height()*130.5/369.0f);
    // 绘制竖直实线
    for(int i = 1; i<gridCount_;i++){
       painter->drawLine(rect.x()+m_width*i,rect.y(),rect.x()+m_width*i,rect.bottom());
    }
    p.setColor(m_dotLineColor);
    p.setWidth(m_dotLineWidth);
    p.setStyle(Qt::DotLine);
    painter->setPen(p);
    // 绘制虚线
    rect = rect.adjusted(0,m_height*132/369.0f,0,0);
    painter->drawLine(rect.x(),rect.y()+rect.height()/2,rect.right(),rect.y()+rect.height()/2);
    for(int i = 0; i<gridCount_;i++){
       painter->drawLine(rect.x()+m_width/2+m_width*i,rect.y(),rect.x()+m_width/2+m_width*i,rect.bottom());
    }
}

void WhitingGrid::adjustWidth(){
       switch (type_) {
        case WhitingGridType::TinWordFormat:
           m_width = m_height * tinWidthHeihtRatio;
           break;
       case WhitingGridType::FourLinesAndThreeGrids:
          m_width = m_height * fourLineThreeGridsWidthHeihtRatio;
          break;
       case WhitingGridType::PinYinTinGrids:
          m_width = m_height * pinYinTinWidthHeightRatio;
          break;
       }
   }
