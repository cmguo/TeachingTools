#include "writinggrid.h"
#include "writinggridcontrol.h"
#include "inkstroke/inkstrokehelper.h"
#include "tesseractocr.h"

#include <core/control.h>
#include <core/resourcerecord.h>
#include <Windows/Controls/inkcanvas.h>

#include <QPainter>
#include <QDebug>
#include <QEvent>
#include <QGraphicsSceneResizeEvent>
#include <QGraphicsProxyWidget>

INKCANVAS_USE_NAMESPACE

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

    leftControlItem = new QGraphicsRectItem();
    leftControlItem->setBrush(QBrush(Qt::transparent));
    leftControlItem->setPen(Qt::NoPen);

    ocrItem = new QGraphicsTextItem(leftControlItem);
    ocrItem->setScale(2.0);
    ocrItem->setPlainText("字");
    ocrItem->setAcceptedMouseButtons(Qt::LeftButton);
    ocrItem->setX((controlItemSize.width()-itemSize.width())/2);

    rightControlItem = new QGraphicsRectItem();
    rightControlItem->setBrush(QBrush(Qt::transparent));
    rightControlItem->setPen(Qt::NoPen);
    addItem = new QGraphicsPixmapItem(rightControlItem);
    addItem->setPixmap(QPixmap(":/teachingtools/icon/icon_plus.png").scaled(itemSize, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    addItem->setAcceptedMouseButtons(Qt::LeftButton);
    decItem = new QGraphicsPixmapItem(rightControlItem);
    decItem->setPixmap(QPixmap(":/teachingtools/icon/icon_minus.png").scaled(itemSize, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    decItem->setAcceptedMouseButtons(Qt::LeftButton);
    addItem->setX((controlItemSize.width()-itemSize.width())/2);
    decItem->setX((controlItemSize.width()-itemSize.width())/2);
    inkItem = new QGraphicsPixmapItem(rightControlItem);
    inkItem->setPixmap(QPixmap(":/teachingtools/icon/icon_ink_checked.png").scaled(itemSize, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    inkItem->setAcceptedMouseButtons(Qt::LeftButton);
    inkItem->setX((controlItemSize.width()-itemSize.width())/2);

    inkEraseItem = new QGraphicsPixmapItem(rightControlItem);
    inkEraseItem->setPixmap(QPixmap(":/teachingtools/icon/icon_eraser_normal.png").scaled(itemSize, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    inkEraseItem->setAcceptedMouseButtons(Qt::LeftButton);
    inkEraseItem->setX((controlItemSize.width() - itemSize.width()) / 2);

    adjustControlItemPos();
    ink = InkStrokeHelper::createInkCanvas(Qt::black, 8, {24, 40});
    ink->SetLimitInputPosition(true);
    ink->SetEditingMode(InkCanvasEditingMode::Ink);
    QObject::connect(ink->Strokes().get(), &StrokeCollection::StrokesChanged,
                     this, &WritingGrid::strokesChanged);
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

    if(watched == rightControlItem && event->type()==QEvent::GraphicsSceneResize){
        QGraphicsSceneResizeEvent *sceneResizeEvent = static_cast<QGraphicsSceneResizeEvent*>(event);
        newScaleSize = sceneResizeEvent->newSize();
        adjustControlItemPos();
        return true;
    }
    bool isMousePressed = event->type()==QEvent::GraphicsSceneMousePress;
    if(event->type() != QEvent::GraphicsSceneMouseRelease&&!isMousePressed){
        return false;
    }

    QGraphicsSceneMouseEvent* mouseEvent = static_cast<QGraphicsSceneMouseEvent*>(event);
    double clickGap = addItem->boundingRect().height() / 2;

    if (watched == leftControlItem) {
        if (mouseEvent->pos().y() > (ocrItem->pos().y() - clickGap) && mouseEvent->pos().y() < (ocrItem->pos().y() + clickGap * 3)) {
            ocr();
            return true;
        }
    }

    if (mouseEvent->pos().y() > (addItem->pos().y() - clickGap) && mouseEvent->pos().y() < (addItem->pos().y() + clickGap * 3)) {
        if(isMousePressed) return true;
        addGrid();
        return true;
    }
    if (decItem->isVisible() && mouseEvent->pos().y() > (decItem->pos().y() - clickGap) && mouseEvent->pos().y() < (decItem->pos().y() + clickGap * 3)) {
        if(isMousePressed) return true;
        decGrid();
        return true;
    }

    if (mouseEvent->pos().y() > (inkItem->pos().y() - clickGap) && mouseEvent->pos().y() < (inkItem->pos().y() + clickGap * 3)) {
        if(isMousePressed) return true;
        inkItem->setPixmap(QPixmap(":/teachingtools/icon/icon_ink_checked.png").scaled(itemSize, Qt::KeepAspectRatio, Qt::SmoothTransformation));
        inkEraseItem->setPixmap(QPixmap(":/teachingtools/icon/icon_eraser_normal.png").scaled(itemSize, Qt::KeepAspectRatio, Qt::SmoothTransformation));
        ink->SetEditingMode(InkCanvasEditingMode::Ink);
        return true;
    }
    if (mouseEvent->pos().y() > (inkEraseItem->pos().y() - clickGap) && mouseEvent->pos().y() < (inkEraseItem->pos().y() + clickGap * 3)) {
        if(isMousePressed) return true;
        inkEraseItem->setPixmap(QPixmap(":/teachingtools/icon/icon_eraser_checked.png").scaled(itemSize, Qt::KeepAspectRatio, Qt::SmoothTransformation));
        inkItem->setPixmap(QPixmap(":/teachingtools/icon/icon_ink_normal.png").scaled(itemSize, Qt::KeepAspectRatio, Qt::SmoothTransformation));
        ink->SetEditingMode(InkCanvasEditingMode::EraseByPoint);
        ink->itemChange(QGraphicsItem::ItemTransformHasChanged, QVariant()); // for update cursor
    }
    return false;
}

QVariant WritingGrid::itemChange(QGraphicsItem::GraphicsItemChange change, const QVariant &value)
{
    switch (change) {
    case ItemVisibleHasChanged:
        leftControlItem->installSceneEventFilter(this);
        rightControlItem->installSceneEventFilter(this);
        WritingGridControl* control = qobject_cast<WritingGridControl*>(WritingGridControl::fromItem(this));
        if (control != nullptr)
            control->sizeChanged();
        break;
    }
    return value;
}

QGraphicsItem *WritingGrid::createLeftControlBar()
{
    return leftControlItem;
}

QGraphicsItem* WritingGrid::createRightControlBar()
{
    return rightControlItem;
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
    RecordMergeScope rs(this);
    if (rs)
        rs.add(MakeFunctionRecord(
                   [this, n = gridCount_] () { setGridCount(n); },
                   [this, n] () { setGridCount(n); }));
    prepareGeometryChange();
    gridCount_ = n;
    WritingGridControl* control = qobject_cast<WritingGridControl*>(WritingGridControl::fromItem(this));
    if (control != nullptr)
        control->sizeChanged();
    adjustControlItemPos();
    adjustInkCanvas();
}

void WritingGrid::addGrid()
{
    setGridCount(gridCount_ + 1);
}

void WritingGrid::decGrid()
{
    if (gridCount_ > 1) {
        setGridCount(gridCount_ - 1);
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
    leftControlItem->setRect(0,0,controlItemSize.width(),newScaleSize.height());
    ocrItem->setY(55 *m_adapterRatio);

    rightControlItem->setRect(0,0,controlItemSize.width(),newScaleSize.height());
    inkEraseItem->setY(55*m_adapterRatio);
    addItem->setY(125 * m_adapterRatio);
    decItem->setVisible(gridCount_ != 1);
    decItem->setY(180 * m_adapterRatio);
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

void WritingGrid::strokesChanged(StrokeCollectionChangedEventArgs &args)
{
    if (timerOcr != 0)
        killTimer(timerOcr);
    timerOcr = startTimer(2000);
}

void WritingGrid::ocr()
{
    static TesseractOcr c;
    QString t = c.detect(ink);
    if (t.isEmpty()) t = "?";
    ocrItem->setPlainText(t);
}

void WritingGrid::timerEvent(QTimerEvent *event)
{
    if (event->timerId() == timerOcr) {
        ocr();
        killTimer(timerOcr);
    }
}

