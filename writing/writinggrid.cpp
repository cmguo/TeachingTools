#include "writinggrid.h"
#include "writinggridcontrol.h"
#include "inkstroke/inkstrokecontrol.h"

#include <QPainter>
#include <QDebug>
#include <QEvent>
#include <QGraphicsSceneResizeEvent>
#include <QGraphicsProxyWidget>
#include <core/control.h>
#include <Windows/Controls/inkcanvas.h>
#include <Windows/Controls/inkevents.h>

WritingGrid::WritingGrid(QGraphicsItem *parent)
{
    WritingGrid(300,WritingGridType::TinWordFormat,parent);
}

WritingGrid::WritingGrid(int h,WritingGridType type,QGraphicsItem * parent):m_height(h),type_(type),QGraphicsItem(parent)
{
    m_realLineColor = QColor(0xCACACA);
    m_dotLineColor = QColor(0xCACACA);
    m_dotLineWidth = 2;
    m_realLineWidth = 3;
    adjustWidth();
    newScaleSize.setWidth(m_width);
    newScaleSize.setHeight(m_height);
    setFlag(QGraphicsItem::ItemIsFocusable, true);
    controlItem = new QGraphicsRectItem();
    controlItem->setBrush(QBrush(Qt::transparent));
    controlItem->setPen(Qt::NoPen);
    addItem = new QGraphicsPixmapItem(controlItem);
    addItem->setPixmap(QPixmap(":/icon/icon/add.svg"));
    addItem->setAcceptedMouseButtons(Qt::LeftButton);
    decItem = new QGraphicsPixmapItem(controlItem);
    decItem->setPixmap(QPixmap(":/icon/icon/remove.svg"));
    decItem->setAcceptedMouseButtons(Qt::LeftButton);
    addItem->setX(2); // icon不居中矫正
    decItem->setX(2);
    adjustControlItemPos();
    ink = InkStrokeControl::createInkCanvas(8);
    QGraphicsProxyWidget * proxy = new QGraphicsProxyWidget(this);
    proxy->setWidget(ink);
    adjustInkCanvas();
}

QRectF WritingGrid::boundingRect() const
{
    return QRectF(0,0,m_width*gridCount_+2*padding,m_height+2*padding);
}

void WritingGrid::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option)
    Q_UNUSED(widget)

    switch (getType()) {
    case WritingGridType::TinWordFormat:
        paintTinWordFormat(painter,option,widget);
        break;
    case WritingGridType::FourLinesAndThreeGrids:
        paintFourLinesAndThreeGrids(painter,option,widget);
        break;
    case WritingGridType::PinYinTinGrids:
        paintPinYinTinGrids(painter,option,widget);
        break;
    }
}

void WritingGrid::paintTinWordFormat(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
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

void WritingGrid::paintFourLinesAndThreeGrids(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
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

void WritingGrid::paintPinYinTinGrids(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
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

bool WritingGrid::sceneEventFilter(QGraphicsItem *watched, QEvent *event)
{

    if(event->type()==QEvent::GraphicsSceneMousePress){
        QGraphicsSceneMouseEvent *mouseEvent = static_cast<QGraphicsSceneMouseEvent*>(event);
        double clickGap = addItem->boundingRect().height()/2;
        if(mouseEvent->pos().y()>(addItem->pos().y()-clickGap) && mouseEvent->pos().y()<(addItem->pos().y()+clickGap*3)){
            addGrid();
            WritingGridControl *control = qobject_cast<WritingGridControl*>(WritingGridControl::fromItem(this));
            if(control != nullptr)
                control->sizeChanged();
            adjustControlItemPos();
            adjustInkCanvas();
            return true;
        }
        if(decItem->isVisible()&&mouseEvent->pos().y()>(decItem->pos().y()-clickGap) && mouseEvent->pos().y()<(decItem->pos().y()+clickGap*3)){
            decGrid();
            WritingGridControl *control = qobject_cast<WritingGridControl*>(WritingGridControl::fromItem(this));
            if(control != nullptr)
                control->sizeChanged();
            adjustControlItemPos();
            adjustInkCanvas();
            return true;
        }

    }
    if(watched == controlItem && event->type()==QEvent::GraphicsSceneResize){
        QGraphicsSceneResizeEvent *sceneResizeEvent = static_cast<QGraphicsSceneResizeEvent*>(event);
        newScaleSize = sceneResizeEvent->newSize();
        adjustControlItemPos();
        return true;
    }
    return false;
}

QVariant WritingGrid::itemChange(QGraphicsItem::GraphicsItemChange change, const QVariant &value)
{
    switch (change) {
    case ItemVisibleHasChanged:
        controlItem->installSceneEventFilter(this);
        break;
    }
    return value;
}

QGraphicsItem* WritingGrid::createControlBar()
{
    return controlItem;
}

void WritingGrid::setDotLineColor(QColor &color){

    this->m_dotLineColor = color;
    this->update(boundingRect());
}

void WritingGrid::setDotLineWidth(int width){
    this->m_dotLineWidth =  width;
    this->update(boundingRect());
}

void WritingGrid::setRealLineWidth(int width){
    this->m_realLineWidth = width;
    this->update(boundingRect());
}

void WritingGrid::setRealLineColor(QColor &color){
    this->m_realLineColor = color;
    this->update(boundingRect());
}

int WritingGrid::getDotLineWidth() const {
    return this->m_dotLineWidth;
}

QColor WritingGrid::getDotLineColor() const{
    return this->m_dotLineColor;
}

int WritingGrid::getRealLineWidth() const{
    return this->m_realLineWidth;
}

QColor WritingGrid::getRealLineColor() const{
    return this->m_realLineColor;
}

WritingGridType WritingGrid::getType(){
    return type_;
}

void WritingGrid::setType(WritingGridType type){
    this->type_ = type;
    adjustWidth();
    adjustControlItemPos();
    adjustInkCanvas();
    update();
}

void WritingGrid::addGrid(){
    gridCount_++;
    update();
}

void WritingGrid::decGrid(){
    if(gridCount_>1)
        gridCount_--;
    update();
}

void WritingGrid::adjustWidth(){
    switch (type_) {
    case WritingGridType::TinWordFormat:
        m_width = m_height * tinWidthHeihtRatio;
        break;
    case WritingGridType::FourLinesAndThreeGrids:
        m_width = m_height * fourLineThreeGridsWidthHeihtRatio;
        break;
    case WritingGridType::PinYinTinGrids:
        m_width = m_height * pinYinTinWidthHeightRatio;
        break;
    }
}

void WritingGrid::adjustControlItemPos()
{
    controlItem->setRect(0,0,controlItemSize.width(),newScaleSize.height());
    if(gridCount_==1){
        // 只展示增加
        decItem->setVisible(false);
        addItem->setY(newScaleSize.height()/2-controlItemSize.height()/2);
        return;
    }
    // 增加 减少都保留
    addItem->setY(newScaleSize.height()*2/5-controlItemSize.height());
    decItem->setVisible(true);
    decItem->setY(newScaleSize.height()*3/5);
}

void WritingGrid::adjustInkCanvas()
{
    ink->setFixedSize(boundingRect().width(),boundingRect().height());
    qDebug()<<"width:"<<boundingRect().width()<<"height:"<<boundingRect().height();
    QGraphicsProxyWidget * proxy = ink->graphicsProxyWidget();
    if (proxy)
        proxy->resize(ink->minimumSize());
}

