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

#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsProxyWidget>
#include <QPen>
#include <QDebug>

static constexpr char const * toolsStr =
        "-full|全屏|NeedUpdate|:/teachingtools/icon/enter_full.svg;"
        "duplex()|双页|Checkable,UnionUpdate|:/teachingtools/icon/double_page.svg;"
        "single()|单页|Checkable,UnionUpdate|:/teachingtools/icon/single_page.svg;"
        ";"
        "scaleUp()||UnionUpdate|:/teachingtools/icon/zoom_in.svg,default;"
        "scaleDown()||UnionUpdate|:/teachingtools/icon/zoom_out.svg,default;"
        "pages||;";

PageBoxItem::PageBoxItem(QGraphicsItem * parent)
    : QGraphicsRectItem(parent)
    , sizeMode_(FixedSize)
{
    setFlags(ItemClipsToShape | ItemClipsChildrenToShape);
    setPen(QPen(Qt::NoPen));
    setBrush(QColor("#FFE2E3E4"));
    setRect({-150, -300, 300, 600});

    document_ = new PageBoxDocItem(this);
    QObject::connect(document_, &PageBoxDocItem::currentPageChanged, this, &PageBoxItem::documentPageChanged);
    QObject::connect(document_, &PageBoxDocItem::sizeChanged, this, &PageBoxItem::documentSizeChanged);

    pageNumber_ = new PageNumberWidget();
    QGraphicsProxyWidget * proxy = new QGraphicsProxyWidget(this);
    toolBar_ = new PageBoxToolBar;
    proxy->setWidget(toolBar_);
    QObject::connect(toolBar_, &PageBoxToolBar::sizeChanged, proxy, [proxy](){
        QPointF pos = -proxy->boundingRect().center();
        proxy->setTransform(QTransform::fromTranslate(pos.x(), pos.y()));
    });
    toolBarProxy_ = proxy;
    QObject::connect(pageNumber_, &PageNumberWidget::pageNumberChanged, this, &PageBoxItem::documentPageChanged);
    setToolsString(toolsStr);

    QObject::connect(document_, &PageBoxDocItem::pageCountChanged, pageNumber_, &PageNumberWidget::setTotal);
}

PageBoxItem::~PageBoxItem()
{
    toolBar_->clear();
    delete pageNumber_;
}

int PageBoxItem::pageNumber()
{
    return document_->curPage();
}

void PageBoxItem::setPageNumber(int n)
{
    document_->goToPage(n);
}

QByteArray PageBoxItem::pageBoxState()
{
    return document_->saveState();
}

void PageBoxItem::setPageBoxState(QByteArray state)
{
    document_->restoreState(state);
}

bool PageBoxItem::selectTest(QPointF const & point)
{
    return (document_->plugin_ == nullptr
                || document_->plugin_->selectTest(document_->pluginItem_->mapFromItem(this, point)))
            && !toolBarProxy_->contains(mapToItem(toolBarProxy_, point));
}

void PageBoxItem::setSizeMode(PageBoxItem::SizeMode mode)
{
    sizeMode_ = mode;
    toolBar_->attachProvider(this);
}

void PageBoxItem::sizeChanged()
{
    document_->rescale();
}

QRectF PageBoxItem::visibleRect() const
{
    return sizeMode_ == LargeCanvas ? QRectF(rect().topLeft(), scene()->sceneRect().size())
                                    : boundingRect();
}

void PageBoxItem::duplex()
{
    document_->setLayoutMode(PageBoxDocItem::Duplex);
}

void PageBoxItem::single()
{
    document_->setLayoutMode(PageBoxDocItem::Single);
}

void PageBoxItem::scaleUp()
{
    document_->stepScale(true);
}

void PageBoxItem::scaleDown()
{
    document_->stepScale(false);
}

void PageBoxItem::exit()
{
    PageBoxControl * control = qobject_cast<PageBoxControl*>(Control::fromItem(this));
    if (control) {
        control->resource()->removeFromPage();
    }
}

void PageBoxItem::getToolButtons(QList<ToolButton *> &buttons, const QList<ToolButton *> &parents)
{
    if (document_->plugin_)
        document_->plugin_->getToolButtons(buttons, parents);
    ToolButtonProvider::getToolButtons(buttons, parents);
    if (parents.empty()) {
        for (ToolButton * & b : buttons) {
            if (b->name() == "pages")
                b = pageNumber_->toolButton();
            if (sizeMode_ != LargeCanvas) {
                if (b->name() == "scaleUp()"
                        || b->name() == "scaleDown()")
                    b = nullptr;
            }
            if (sizeMode_ != MatchContent) {
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
        button->setChecked(document_->layoutMode() == PageBoxDocItem::Single);
    } else if (button->name() == "scaleUp()") {
        button->setEnabled(document_->canStepScale(true));
    } else if (button->name() == "scaleDown()") {
        button->setEnabled(document_->canStepScale(false));
    }
}

void PageBoxItem::handleToolButton(const QList<ToolButton *> &buttons)
{
    ToolButton * button = buttons.back();
    if (button->name() == "full") {
        if (button->isChecked()) {
            button->setChecked(false);
            button->setText("全屏");
            button->setIcon(":/teachingtools/icon/enter_full.svg");
        } else {
            button->setChecked(true);
            button->setText("缩小");
            button->setIcon(":/teachingtools/icon/exit_full.svg");
        }
    }
    if (document_->plugin_)
        document_->plugin_->handleToolButton(buttons);
    ToolButtonProvider::handleToolButton(buttons);
}

void PageBoxItem::documentPageChanged(int page)
{
    if (sender() == document_)
        pageNumber_->setNumber(page);
    else
        document_->goToPage(page);
}

void PageBoxItem::documentSizeChanged(const QSizeF &pageSize2)
{
    if (sizeMode_ == FixedSize) {
        return;
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
}

QSizeF PageBoxItem::calcSize(QSizeF const &pageSize2)
{
    QRectF rect = this->rect();
    if (sizeMode_ == LargeCanvas) {
        PageBoxControl * control = qobject_cast<PageBoxControl*>(Control::fromItem(this));
        if (control->flags() & Control::RestoreSession)
            return rect.size();
        QSizeF docSize = document_->documentSize();
        document_->rescale();
        document_->transferToManualScale();
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
        document_->moveBy(d.x(), d.y());
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
    if (document_->scaleMode() == PageBoxDocItem::WholePage) {
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
        document_->moveBy(0, d);
    } else {
        if (rect.left() + d > vrect.left())
            d = rect.left() > vrect.left() ? 0 : vrect.left() - rect.left();
        else if (rect.right() + d < vrect.right())
            d = rect.right() < vrect.right() ? 0 : vrect.right() - rect.right();
        document_->moveBy(d, 0);
    }
}
