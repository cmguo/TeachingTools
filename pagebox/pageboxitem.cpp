#include "pageboxitem.h"
#include "pageboxdocitem.h"
#include "pageboxcontrol.h"
#include "pageboxtoolbar.h"
#include "pagenumberwidget.h"
#include "pageboxplugin.h"

#include <views/toolbarwidget.h>
#include <core/control.h>
#include <core/resourceview.h>
#include <core/resourcetransform.h>
#include <core/controltransform.h>

#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsProxyWidget>
#include <QPen>
#include <QDebug>

static constexpr char const * toolsStr =
        "scaleUp()||UnionUpdate|:/teachingtools/icon/zoom_in.svg,default;"
        "scaleDown()||UnionUpdate|:/teachingtools/icon/zoom_out.svg,default;"
        "|;"
        "duplex()|双页|Checkable,UnionUpdate|:/teachingtools/icon/double_page.svg;"
        "single()|单页|Checkable,UnionUpdate|:/teachingtools/icon/single_page.svg;"
        "pages||;"
        "feedback()|反馈||:/teachingtools/icon/icon_feedback.svg;";

PageBoxItem::PageBoxItem(QGraphicsItem * parent)
    : QGraphicsRectItem(parent)
    , pagesMode_(Paper)
    , sizeMode_(FixedSize)
    , scaleMode_(FitLayout)
    , scaleInterval_(1.2)
    , scaleLevel_(0)
    , maxScaleLevel_(0)
{
    setFlags(ItemClipsToShape | ItemClipsChildrenToShape);
    setPen(QPen(Qt::NoPen));
    setBrush(QColor("#FFE2E3E4"));
    setRect({-150, -300, 300, 600});

    document_ = new PageBoxDocItem(this);
    QObject::connect(document_, &PageBoxDocItem::pageSize2Changed,
                     this, &PageBoxItem::documentSizeChanged);
    transform_ = new ResourceTransform(this);
    document_->setTransformations({new ControlTransform(*transform_)});
    QObject::connect(document_, &PageBoxDocItem::requestPosition,
                     this, &PageBoxItem::setDocumentPosition);
    toolBar_ = nullptr;
    toolBarProxy_ = nullptr;
    setToolsString(toolsStr);
}

PageBoxItem::~PageBoxItem()
{
    if (toolBar_)
        toolBar_->attachProvider(nullptr);
}

int PageBoxItem::pageNumber()
{
    return document_->curPage();
}

void PageBoxItem::setPageNumber(int n)
{
    document_->goToPage(n);
}

struct TransformData
{
    TransformData() {}
    TransformData(QTransform const & t)
    {
        scale = t.m11();
        offset.setX(t.dx());
        offset.setY(t.dy());
    }
    QTransform toQTransform() const
    {
        return QTransform::fromTranslate(offset.x(), offset.y()).scale(scale, scale);
    }
    qreal scale;
    QPointF offset;
};

QByteArray PageBoxItem::pageBoxState()
{
    qDebug() << "PageBoxItem saveState" << transform_->transform();
    pos_ = transform_->offset();
    char * p1 = reinterpret_cast<char *>(&pagesMode_);
    char * p2 = reinterpret_cast<char *>(&start_);
    QByteArray data(p1, static_cast<int>(p2 - p1));
    if (document_->transformations().empty()) {
        data.append(1);
        TransformData d(document_->transform());
        data.append(reinterpret_cast<char *>(&d), sizeof(d));
    } else {
        data.append('\000');
    }
    data.append(document_->saveState());
    return data;
}

void PageBoxItem::setPageBoxState(QByteArray state)
{
    char * p1 = reinterpret_cast<char *>(&pagesMode_);
    char * p2 = reinterpret_cast<char *>(&start_);
    int n = static_cast<int>(p2 - p1);
    memcpy(p1, state.data(), static_cast<size_t>(n));
    if (state[static_cast<int>(n)]) {
        TransformData d;
        memcpy(reinterpret_cast<char *>(&d), state.data() + n + 1, sizeof(d));
        document_->setTransform(d.toQTransform());
        document_->setTransformations({});
        n += sizeof(d);
    }
    ++n;
    document_->restoreState(state.mid(n));
}

bool PageBoxItem::selectTest(QPointF const & point)
{
    for (PageBoxPlugin * plugin : document_->plugins()) {
        if (!plugin->selectTest(plugin->item()->mapFromItem(this, point)))
            return false;
    }
    return !toolBarProxy_->contains(mapToItem(toolBarProxy_, point));
}

void PageBoxItem::setPagesMode(PageBoxItem::PagesMode mode)
{
    pagesMode_ = mode;
    switch (mode) {
    case Paper:
        document_->setLayoutMode(PageBoxDocItem::Continuous);
        document_->setDirection(PageBoxDocItem::Vertical);
        document_->setPadding(30);
        setScaleMode(WholePage);
        break;
    case Book:
        document_->setLayoutMode(PageBoxDocItem::Duplex);
        document_->setDirection(PageBoxDocItem::Horizontal);
        setScaleMode(WholePage);
        break;
    }
}

void PageBoxItem::setSizeMode(PageBoxItem::SizeMode mode)
{
    sizeMode_ = mode;
    if (mode != LargeCanvas) {
        toolBar_ = new PageBoxToolBar;
        toolBarProxy_ = toolBar_->toGraphicsProxy(this);
        toolBar_->attachProvider(this);
    }
}

void PageBoxItem::sizeChanged()
{
    rescale();
}

QRectF PageBoxItem::visibleRect() const
{
    return sizeMode_ == LargeCanvas ? QRectF(rect().topLeft(), scene()->sceneRect().size())
                                    : boundingRect();
}

void PageBoxItem::setPlugin(PageBoxPlugin *plugin)
{
    attachSubProvider(plugin, true);
    if (plugin)
        document_->addPlugin(plugin);
    else if (!document_->plugins().empty())
        document_->removePlugin(document_->plugins().first());
}

void PageBoxItem::duplex()
{
    document_->setLayoutMode(PageBoxDocItem::Duplex);
}

void PageBoxItem::single()
{
    document_->setLayoutMode(PageBoxDocItem::DuplexSingle);
}

void PageBoxItem::scaleUp()
{
    stepScale(true);
}

void PageBoxItem::scaleDown()
{
    stepScale(false);
}

qreal PageBoxItem::scale() const
{
    return transform_->scale().m11();
}

void PageBoxItem::setScaleMode(ScaleMode mode)
{
    if (scaleMode_ == mode)
        return;
    scaleMode_ = mode;
    rescale();
}

void PageBoxItem::setManualScale(qreal scale, bool changeMode)
{
    if (scaleMode_ == ManualScale && qFuzzyIsNull(manualScale_ - scale))
        return;
    manualScale_ = scale;
    if (changeMode)
        scaleMode_ = ManualScale;
    if (scaleMode_ == ManualScale)
        rescale();
}

void PageBoxItem::transferToManualScale()
{
    QRectF vrect = visibleRect();
    qreal sw = vrect.width() / document_->pageSize().width();
    qreal sh = vrect.height() / document_->pageSize().height();
    qreal sMin = qMin(sh, sw);
    qreal sMax = qMax(sh, sw);
    qreal d = sMax / sMin;
    maxScaleLevel_ = 1;
    while (d >= 1.44) {
        maxScaleLevel_ *= 2;
        d = sqrt(d);
    }
    scaleInterval_ = d;
    manualScale_ = scale();
    scaleLevel_ = 0;
    sMin *= scaleInterval_;
    while (sMin < manualScale_) {
        sMin *= scaleInterval_;
        ++scaleLevel_;
    }
    scaleMode_ = ManualScale;
}

void PageBoxItem::stepScale(bool up)
{
    if (up) {
        if (scaleLevel_ < maxScaleLevel_) {
            ++scaleLevel_;
            setManualScale(scale() * scaleInterval_);
        }
    } else {
        if (scaleLevel_ > 0) {
            --scaleLevel_;
            setManualScale(scale() / scaleInterval_);
        }
    }
}

bool PageBoxItem::canStepScale(bool up)
{
    if (up)
        return scaleLevel_ < maxScaleLevel_;
    else
        return scaleLevel_ > 0;
}

void PageBoxItem::stepMiddleScale()
{
    qreal s = scale();
    for (; scaleLevel_ < maxScaleLevel_ / 2; ++scaleLevel_)
        s *= scaleInterval_;
    for (; scaleLevel_ > maxScaleLevel_ / 2; --scaleLevel_)
        s /= scaleInterval_;
    setManualScale(s);
    QPointF off = transform_->offset();
    off.setY(boundingRect().top());
    transform_->translateTo(off);
}

void PageBoxItem::rescale()
{
    if (document_->boundingRect().isEmpty())
        return;
    QRectF vrect = visibleRect();
    qreal s = 1.0;
    if (scaleMode_ == ManualScale) {
        s = manualScale_;
    } else {
        s = document_->requestScale(vrect.size(), scaleMode_ == WholePage);
    }
    QRectF rect = document_->rect();
    transform_->scaleKeepToCenter(vrect, rect, s);
}

ResourceTransform * PageBoxItem::detachTransform()
{
    if (!document_->transformations().empty()) { // else already detach in restore
        document_->setTransform(transform_->transform());
        document_->setTransformations({});
    }
    // use QueuedConnection to handle after attaching/canvas transform changed
    QObject::connect(transform_, &ResourceTransform::changed,
                     this, &PageBoxItem::onTransformChanged, Qt::QueuedConnection);
    return transform_;
}

void PageBoxItem::restorePosition()
{
    if (!pos_.isNull()) {
        transform_->translateTo(pos_);
        pos_ = QPointF();
        qDebug() << "PageBoxItem restorePosition" << transform_->transform();
    }
}

void PageBoxItem::exit()
{
    PageBoxControl * control = qobject_cast<PageBoxControl*>(Control::fromItem(this));
    if (control) {
        control->resource()->removeFromPage();
    }
}

void PageBoxItem::getToolButtons(QList<ToolButton *> &buttons, ToolButton *parent)
{
    ToolButtonProvider::getToolButtons(buttons, parent);
    if (parent == nullptr) {
        for (ToolButton * & b : buttons) {
            if (b->name() == "pages")
                b = document_->pageNumberWidget()->toolButton();
            if (sizeMode_ != LargeCanvas) {
                if (b->name() == "scaleUp()"
                        || b->name() == "scaleDown()")
                    b = nullptr;
            }
            if (pagesMode_ != Book) {
                if (b && (b->name() == "duplex()"
                        || b->name() == "single()"))
                    b = nullptr;
            }
        }
        buttons.removeAll(nullptr);
    }
    if (buttons.endsWith(&ToolButton::SPLITTER))
        buttons.pop_back();
}

void PageBoxItem::updateToolButton(ToolButton *button)
{
    if (button->name() == "duplex()") {
        button->setChecked(document_->layoutMode() == PageBoxDocItem::Duplex);
    } else if (button->name() == "single()") {
        button->setChecked(document_->layoutMode() == PageBoxDocItem::DuplexSingle);
    } else if (button->name() == "scaleUp()") {
        button->setEnabled(canStepScale(true));
    } else if (button->name() == "scaleDown()") {
        button->setEnabled(canStepScale(false));
    } else {
        ToolButtonProvider::updateToolButton(button);
    }
}

void PageBoxItem::documentSizeChanged(const QSizeF &pageSize2)
{
    if (sizeMode_ == FixedSize) {
        return;
    }
    if (sizeMode_ == LargeCanvas) {
        PageBoxControl * control = qobject_cast<PageBoxControl*>(Control::fromItem(this));
        if (control->flags() & (Control::RestoreSession | Control::LoadFinished)) {
            rescale();
            return;
        }
    }
    QSizeF size2 = calcSize(pageSize2);
    QRectF rect(QPointF(0, 0), size2);
    rect.moveCenter(QPointF(0, 0));
    setRect(rect);
    qDebug() << "PageBoxItem documentSizeChanged" << rect;
    PageBoxControl * control = qobject_cast<PageBoxControl*>(Control::fromItem(this));
    if (control) {
        control->sizeChanged();
    }
    rescale();
}

void PageBoxItem::setDocumentPosition(const QPointF &pos)
{
    QPointF p(pos);
    QPointF tl = boundingRect().topLeft();
    if (p.x() < 0)
        p.setX(transform_->translate().dx());
    else
        p.setX(tl.x() - p.x() * scale());
    if (p.y() < 0)
        p.setY(transform_->translate().dy());
    else
        p.setY(tl.y() - p.y() * scale());
    transform_->translateTo(p);
}

void PageBoxItem::onTransformChanged()
{
    qDebug() << "PageBoxItem onTransformChanged" << transform_->transform();
    QPointF center = mapFromScene(scene()->sceneRect()).boundingRect().center();
    document_->visiblePositionHint(this, center);
}

QSizeF PageBoxItem::calcSize(QSizeF const &pageSize2)
{
    QRectF rect = this->rect();
    if (sizeMode_ == LargeCanvas) {
        QSizeF docSize = document_->documentSize();
        rescale();
        transferToManualScale();
        if (document_->direction() == PageBoxDocItem::Horizontal) {
            qreal s = pageSize2.width() / rect.width();
            return QSizeF(docSize.width() / s, rect.height());
        } else {
            qreal s = pageSize2.height() / rect.height();
            return QSizeF(rect.width(), docSize.height() / s);
        }
    } else {
        qreal s = document_->direction() == PageBoxDocItem::Horizontal
                ? rect.height() / pageSize2.height() : rect.width() / pageSize2.width();
        return pageSize2 * s;
    }
}

void PageBoxItem::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    if (!document_->contains(mapToItem(document_, event->pos()))) {
        QGraphicsRectItem::mousePressEvent(event);
        return;
    }
    start_ = event->pos();
    type_ = 1;
}

void PageBoxItem::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    if (type_ == 0) {
        QGraphicsRectItem::mouseMoveEvent(event);
        return;
    }
    QPointF pt = event->pos();
    QPointF d = pt - start_;
    QRectF vrect = rect();
    QRectF rect = mapFromItem(document_, document_->boundingRect()).boundingRect();
    switch (type_) {
    case 1:
    case 2:
        if (rect.left() + d.x() > vrect.left())
            d.setX(rect.left() > vrect.left() ? 0 : vrect.left() - rect.left());
        else if (rect.right() + d.x() < vrect.right())
            d.setX(rect.right() < vrect.right() ? 0 : vrect.right() - rect.right());
        if (rect.top() + d.y() > vrect.top())
            d.setY(rect.top() > vrect.top() ? 0 : vrect.top() - rect.top());
        else if (rect.bottom() + d.y() < vrect.bottom())
            d.setY(rect.bottom() < vrect.bottom() ? 0 : vrect.bottom() - rect.bottom());
        transform_->translate({d.x(), d.y()});
        type_ = 2;
        break;
    }
    start_ = pt;
}

void PageBoxItem::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    if (type_ == 0) {
        QGraphicsRectItem::mouseReleaseEvent(event);
        return;
    }
    switch (type_) {
    case 1:
        document_->hit(mapToItem(document_, start_));
        break;
    }
    type_ = 0;
}

void PageBoxItem::wheelEvent(QGraphicsSceneWheelEvent *event)
{
    if (sizeMode_ == LargeCanvas || !document_->contains(mapToItem(document_, event->pos()))) {
        QGraphicsRectItem::wheelEvent(event);
        return;
    }
    QRectF vrect = rect();
    QRectF rect = mapFromItem(document_, document_->boundingRect()).boundingRect();
    qreal d = event->delta();
    if (scaleMode() == WholePage) {
        if (d > 0)
            document_->previousPage();
        else
            document_->nextPage();
        return;
    }
    if (document_->direction() == PageBoxDocItem::Vertical) {
        if (rect.top() + d > vrect.top())
            d = rect.top() > vrect.top() ? 0 : vrect.top() - rect.top();
        else if (rect.bottom() + d < vrect.bottom())
            d = rect.bottom() < vrect.bottom() ? 0 : vrect.bottom() - rect.bottom();
        transform_->translate({0, d});
    } else {
        if (rect.left() + d > vrect.left())
            d = rect.left() > vrect.left() ? 0 : vrect.left() - rect.left();
        else if (rect.right() + d < vrect.right())
            d = rect.right() < vrect.right() ? 0 : vrect.right() - rect.right();
        transform_->translate({d, 0});
    }
}
