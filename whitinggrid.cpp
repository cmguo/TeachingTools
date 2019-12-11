#include "whitinggrid.h"

#include <QPainter>
#include <QDebug>
#include <QEvent>
#include <QGraphicsSceneResizeEvent>
#include <QGraphicsProxyWidget>
#include <core/control.h>
#include <control/showboardcontrol.h>
#include <Windows/Controls/inkcanvas.h>

WhitingGrid::WhitingGrid(QGraphicsItem *parent)
{
    WhitingGrid(300,WhitingGridType::TinWordFormat,parent);
}

WhitingGrid::WhitingGrid(int h,WhitingGridType type,QGraphicsItem * parent):m_height(h),type_(type),QGraphicsItem(parent)
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
    ink = new InkCanvas;
    ink->setStyleSheet("background:#00000000");
    ink->SetEditingMode(InkCanvasEditingMode::Ink);
    QGraphicsProxyWidget * proxy = new QGraphicsProxyWidget(this);
    proxy->setWidget(ink);
    adjustInkCanvas();
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

bool WhitingGrid::sceneEventFilter(QGraphicsItem *watched, QEvent *event)
{

    if(event->type()==QEvent::GraphicsSceneMousePress){
        QGraphicsSceneMouseEvent *mouseEvent = static_cast<QGraphicsSceneMouseEvent*>(event);
        double clickGap = addItem->boundingRect().height()/2;
        if(mouseEvent->pos().y()>(addItem->pos().y()-clickGap) && mouseEvent->pos().y()<(addItem->pos().y()+clickGap*3)){
            addGrid();
            ShowBoardControl *control = qobject_cast<ShowBoardControl*>(ShowBoardControl::fromItem(this));
            if(control != nullptr)
                control->sizeChanged();
            adjustControlItemPos();
            adjustInkCanvas();
            return true;
        }
        if(decItem->isVisible()&&mouseEvent->pos().y()>(decItem->pos().y()-clickGap) && mouseEvent->pos().y()<(decItem->pos().y()+clickGap*3)){
            decGrid();
            ShowBoardControl *control = qobject_cast<ShowBoardControl*>(ShowBoardControl::fromItem(this));
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

QVariant WhitingGrid::itemChange(QGraphicsItem::GraphicsItemChange change, const QVariant &value)
{
    switch (change) {
    case ItemVisibleHasChanged:
        controlItem->installSceneEventFilter(this);
        break;
    }
    return value;
}

QGraphicsItem* WhitingGrid::createControlBar()
{
    return controlItem;
}

void WhitingGrid::setDotLineColor(QColor &color){

    this->m_dotLineColor = color;
    this->update(boundingRect());
}

void WhitingGrid::setDotLineWidth(int width){
    this->m_dotLineWidth =  width;
    this->update(boundingRect());
}

void WhitingGrid::setRealLineWidth(int width){
    this->m_realLineWidth = width;
    this->update(boundingRect());
}

void WhitingGrid::setRealLineColor(QColor &color){
    this->m_realLineColor = color;
    this->update(boundingRect());
}

int WhitingGrid::getDotLineWidth() const {
    return this->m_dotLineWidth;
}

QColor WhitingGrid::getDotLineColor() const{
    return this->m_dotLineColor;
}

int WhitingGrid::getRealLineWidth() const{
    return this->m_realLineWidth;
}

QColor WhitingGrid::getRealLineColor() const{
    return this->m_realLineColor;
}

WhitingGridType WhitingGrid::getType(){
    return type_;
}

void WhitingGrid::setType(WhitingGridType type){
    this->type_ = type;
    adjustWidth();
    adjustControlItemPos();
    adjustInkCanvas();
    update();
}

void WhitingGrid::addGrid(){
    gridCount_++;
    update();
}

void WhitingGrid::decGrid(){
    if(gridCount_>1)
        gridCount_--;
    update();
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

void WhitingGrid::adjustControlItemPos()
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

void WhitingGrid::adjustInkCanvas()
{
    ink->setFixedSize(boundingRect().width(),boundingRect().height());
    qDebug()<<"width:"<<boundingRect().width()<<"height:"<<boundingRect().height();
    QGraphicsProxyWidget * proxy = ink->graphicsProxyWidget();
    if (proxy)
        proxy->resize(ink->minimumSize());
}


