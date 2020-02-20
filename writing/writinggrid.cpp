#include "writinggrid.h"
#include "writinggridcontrol.h"
#include "inkstroke/inkstrokehelper.h"

#include <QPainter>
#include <QDebug>
#include <QEvent>
#include <QGraphicsSceneResizeEvent>
#include <QGraphicsProxyWidget>
#include <core/control.h>
#include <Windows/Controls/inkcanvas.h>
#include <Windows/Controls/inkevents.h>

WritingGrid::WritingGrid(QGraphicsItem *parent)
    : WritingGrid(300,WritingGridType::TinWordFormat,parent)
{
}

WritingGrid::WritingGrid(int h,WritingGridType type,QGraphicsItem * parent)
    : QGraphicsObject(parent)
    , m_height(h)
    , type_(type)
{
    m_realLineColor = QColor(0xC3A4A4);
    m_dotLineColor = QColor(0xC3A4A4);
    m_dotLineWidth = 1;
    m_realLineWidth = 2;
    adjustWidth();
    newScaleSize.setWidth(m_width);
    newScaleSize.setHeight(m_height);
    setFlag(QGraphicsItem::ItemIsFocusable, true);
    setFlags(ItemClipsToShape | ItemClipsChildrenToShape);
    controlItem = new QGraphicsRectItem();
    controlItem->setBrush(QBrush(Qt::transparent));
    controlItem->setPen(Qt::NoPen);
    addItem = new QGraphicsPixmapItem(controlItem);
    addItem->setPixmap(QPixmap(":/teachingtools/icon/icon_plus.png").scaled(itemSize, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    addItem->setAcceptedMouseButtons(Qt::LeftButton);
    decItem = new QGraphicsPixmapItem(controlItem);
    decItem->setPixmap(QPixmap(":/teachingtools/icon/icon_minus.png").scaled(itemSize, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    decItem->setAcceptedMouseButtons(Qt::LeftButton);
    addItem->setX((controlItemSize.width()-itemSize.width())/2);
    decItem->setX((controlItemSize.width()-itemSize.width())/2);
    inkItem = new QGraphicsPixmapItem(controlItem);
    inkItem->setPixmap(QPixmap(":/teachingtools/icon/icon_ink_checked.png").scaled(itemSize, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    inkItem->setAcceptedMouseButtons(Qt::LeftButton);
    inkItem->setX((controlItemSize.width()-itemSize.width())/2);

    inkEraseItem = new QGraphicsPixmapItem(controlItem);
    inkEraseItem->setPixmap(QPixmap(":/teachingtools/icon/icon_eraser_normal.png").scaled(itemSize, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    inkEraseItem->setAcceptedMouseButtons(Qt::LeftButton);
    inkEraseItem->setX((controlItemSize.width() - itemSize.width()) / 2);
    adjustControlItemPos();
    ink = InkStrokeHelper::createInkCanvas(Qt::black, 8, {24, 40});
    ink->SetLimitInputPosition(true);
    ink->SetEditingMode(InkCanvasEditingMode::Ink);
    //QGraphicsProxyWidget * proxy = new QGraphicsProxyWidget(this);
    //proxy->setWidget(ink);
    ink->setParentItem(this);
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
    case WritingGridType::PinYin:
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
    QVector<qreal> dashes;
    qreal space = 10;
    dashes << 10 << space << 10 << space;
    p.setDashPattern(dashes);
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
    painter->drawLine(rect.x(), rect.bottom(), rect.right(), rect.bottom()); // 第1、4条线宽度相同
    p.setWidth(1);
    painter->setPen(p);
    painter->drawLine(rect.x(),rect.y()+rect.height()/3,rect.right(),rect.y()+rect.height()/3);
    if(getType()==WritingGridType::FourLinesAndThreeGrids){
        p.setWidth(m_realLineWidth);
        painter->setPen(p);
    }
    painter->drawLine(rect.x(),rect.y()+rect.height()*2/3,rect.right(),rect.y()+rect.height()*2/3);
   

}

void WritingGrid::paintPinYinTinGrids(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{ // 绘制拼音田字格
    QPen p = QPen(m_realLineColor,m_realLineWidth);
    p.setJoinStyle(Qt::PenJoinStyle::MiterJoin);
    painter->setPen(p);
    QRectF rect = boundingRect().adjusted(padding,padding,-padding,-padding);
    painter->setBrush(Qt::white);
    painter->drawRect(rect);
    p.setWidth(1);
    painter->setPen(p);
    painter->drawLine(rect.x(),rect.y()+rect.height()*44.5/369.0f,rect.right(),rect.y()+rect.height()*44.5/369.0f);
    painter->drawLine(rect.x(),rect.y()+rect.height()*87.5/369.0f,rect.right(),rect.y()+rect.height()*87.5/369.0f);
	p.setWidth(m_realLineWidth);
	painter->setPen(p);
    painter->drawLine(rect.x(), rect.y() + rect.height() * 130.5 / 369.0f, rect.right(), rect.y() + rect.height() * 130.5 / 369.0f);
    // 绘制竖直实线
    for(int i = 1; i<gridCount_;i++){
        painter->drawLine(rect.x()+m_width*i,rect.y(),rect.x()+m_width*i,rect.bottom());
    }
    p.setColor(m_dotLineColor);
    p.setWidth(m_dotLineWidth);
	QVector<qreal> dashes;
	qreal space = 10;
	dashes << 10 << space << 10 << space;
	p.setDashPattern(dashes);
    painter->setPen(p);
    // 绘制虚线
    rect = rect.adjusted(0,m_height*132/369.0f,0,0);
    painter->drawLine(rect.x(),rect.y()+rect.height()/2,rect.right(),rect.y()+rect.height()/2);
    for(int i = 0; i<gridCount_;i++){
        painter->drawLine(rect.x()+m_width/2+m_width*i,rect.y(),rect.x()+m_width/2+m_width*i,rect.bottom());
    }
}

bool WritingGrid::sceneEventFilter(QGraphicsItem* watched, QEvent* event)
{

    if (event->type() == QEvent::GraphicsSceneMousePress) {
        QGraphicsSceneMouseEvent* mouseEvent = static_cast<QGraphicsSceneMouseEvent*>(event);
        double clickGap = addItem->boundingRect().height() / 2;
        if (mouseEvent->pos().y() > (addItem->pos().y() - clickGap) && mouseEvent->pos().y() < (addItem->pos().y() + clickGap * 3)) {
            addGrid();
            WritingGridControl* control = qobject_cast<WritingGridControl*>(WritingGridControl::fromItem(this));
            if (control != nullptr)
                control->sizeChanged();
            adjustControlItemPos();
            adjustInkCanvas();
            return true;
        }
        if (decItem->isVisible() && mouseEvent->pos().y() > (decItem->pos().y() - clickGap) && mouseEvent->pos().y() < (decItem->pos().y() + clickGap * 3)) {
            decGrid();
            WritingGridControl* control = qobject_cast<WritingGridControl*>(WritingGridControl::fromItem(this));
            if (control != nullptr)
                control->sizeChanged();
            adjustControlItemPos();
            adjustInkCanvas();
            return true;
        }

        if (mouseEvent->pos().y() > (inkItem->pos().y() - clickGap) && mouseEvent->pos().y() < (inkItem->pos().y() + clickGap * 3)) {
            inkItem->setPixmap(QPixmap(":/teachingtools/icon/icon_ink_checked.png").scaled(itemSize, Qt::KeepAspectRatio, Qt::SmoothTransformation));
            inkEraseItem->setPixmap(QPixmap(":/teachingtools/icon/icon_eraser_normal.png").scaled(itemSize, Qt::KeepAspectRatio, Qt::SmoothTransformation));
            ink->SetEditingMode(InkCanvasEditingMode::Ink);
            return true;
        }
        if (mouseEvent->pos().y() > (inkEraseItem->pos().y() - clickGap) && mouseEvent->pos().y() < (inkEraseItem->pos().y() + clickGap * 3)) {
            inkEraseItem->setPixmap(QPixmap(":/teachingtools/icon/icon_eraser_checked.png").scaled(itemSize, Qt::KeepAspectRatio, Qt::SmoothTransformation));
            inkItem->setPixmap(QPixmap(":/teachingtools/icon/icon_ink_normal.png").scaled(itemSize, Qt::KeepAspectRatio, Qt::SmoothTransformation));
            ink->SetEditingMode(InkCanvasEditingMode::EraseByPoint);
            ink->itemChange(QGraphicsItem::ItemTransformHasChanged, QVariant()); // for update cursor
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
        WritingGridControl* control = qobject_cast<WritingGridControl*>(WritingGridControl::fromItem(this));
		if (control != nullptr)
			control->sizeChanged();
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

QSharedPointer<StrokeCollection> WritingGrid::strokes()
{
    return ink->Strokes();
}

void WritingGrid::setStrokes(QSharedPointer<StrokeCollection> strokes)
{
    ink->SetStrokes(strokes);
}

int WritingGrid::gridCount() const
{
    return gridCount_;
}

void WritingGrid::setGridCount(int n)
{
    prepareGeometryChange();
    gridCount_ = n;
    adjustControlItemPos();
    adjustInkCanvas();
}

void WritingGrid::addGrid(){
    gridCount_++;
}

void WritingGrid::decGrid(){
    if(gridCount_>1) {
        prepareGeometryChange();
        gridCount_--;
    }
}

void WritingGrid::adjustWidth(){
    m_width = m_height;
    switch (type_) {
    case WritingGridType::TinWordFormat:
        m_height = m_width /tinWidthHeihtRatio;
        m_adapterRatio = m_width / 300.0f;
        break;
    case WritingGridType::FourLinesAndThreeGrids:
    case WritingGridType::PinYin:
        m_width = m_height * fourLineThreeGridsWidthHeihtRatio;
        m_adapterRatio = m_height / 300.0f;
        break;
    case WritingGridType::PinYinTinGrids:
        m_height = m_width /  pinYinTinWidthHeightRatio;
        m_adapterRatio = m_width / 300.0f;
        break;
    }
    controlItemSize.setWidth(controlItemSize.width() * m_adapterRatio);
    controlItemSize.setHeight(controlItemSize.height() * m_adapterRatio);
	itemSize.setWidth(itemSize.width() * m_adapterRatio);
    itemSize.setHeight(itemSize.height() * m_adapterRatio);
}

void WritingGrid::adjustControlItemPos()
{
    controlItem->setRect(0,0,controlItemSize.width(),newScaleSize.height());
	inkEraseItem->setY(newScaleSize.height() / 4);
	addItem->setY(newScaleSize.height() * 3 / 4 - itemSize.height());
	decItem->setVisible(gridCount_ != 1);
	decItem->setY(newScaleSize.height() - itemSize.height());
}

void WritingGrid::adjustInkCanvas()
{
    //ink->setFixedSize(boundingRect().width(),boundingRect().height());
    qDebug()<<"width:"<<boundingRect().width()<<"height:"<<boundingRect().height();
    ink->SetRenderSize(boundingRect().size());
    //QGraphicsProxyWidget * proxy = ink->graphicsProxyWidget();
    //if (proxy)
    //    proxy->resize(ink->minimumSize());
}

