#include "pageboxcontrol.h"
#include "pageboxitem.h"
#include "pageboxdocitem.h"
#include "qpropertybindings.h"
#include "inkpadplugin.h"

#include <core/resource.h>
#include <core/resourceview.h>
#include <core/resourcetransform.h>
#include <core/controltransform.h>

#include <qcomponentcontainer.h>

#include <QStandardItemModel>
#include <QGraphicsScene>
#include <QUrl>
#include <QDir>

PageBoxControl::PageBoxControl(ResourceView * res)
    : Control(res, {KeepAspectRatio}, {CanRotate})
{
}

QGraphicsItem* PageBoxControl::create(ResourceView *res)
{
    (void)res;
    PageBoxItem * item = new PageBoxItem;
    if (res_->flags().testFlag(ResourceView::LargeCanvas)) {
        item->setAcceptedMouseButtons(Qt::NoButton);
    }
    return item;
}

class StaticTransform : public QGraphicsTransform
{
public:
    StaticTransform(QTransform const & transform, QObject * parent)
        : QGraphicsTransform(parent)
        , transform_(transform)
    {
    }
    void setTransform(QTransform const & transform)
    {
        transform_ = transform;
    }
    virtual void applyTo(QMatrix4x4 *matrix) const override
    {
        //*matrix = transform_.toAffine()* *matrix ;
        *matrix *= transform_.toAffine();
    }
private:
    QTransform transform_;
};

void PageBoxControl::attaching()
{
    if (flags_ & RestoreSession)
        return;
    PageBoxItem * item = static_cast<PageBoxItem *>(item_);
    PageBoxDocItem * doc = item->document();
    if (res_->flags().testFlag(ResourceView::LargeCanvas)) {
        doc->setLayoutMode(PageBoxDocItem::Continuous);
        doc->setDirection(PageBoxDocItem::Vertical);
        doc->setScaleMode(PageBoxDocItem::WholePage);
        doc->setPadding(30);
    } else {
        doc->setLayoutMode(PageBoxDocItem::Duplex);
        doc->setDirection(PageBoxDocItem::Horizontal);
        doc->setScaleMode(PageBoxDocItem::WholePage);
    }
    //doc->setPlugin(new InkPadPlugin);
}

void PageBoxControl::attached()
{
    PageBoxItem * item = static_cast<PageBoxItem *>(item_);
    if (res_->flags().testFlag(ResourceView::LargeCanvas)) {
        // attach to canvas transform
        QGraphicsItem* canvas = item_->parentItem()->parentItem();
        QGraphicsTransform * ct = Control::fromItem(canvas)->transform();
        ControlTransform * ct1 = new ControlTransform(static_cast<ControlTransform*>(ct), true, true, true);
        QPointF pos(0, item_->scene()->sceneRect().bottom() - 30);
        StaticTransform* ct2 = new StaticTransform(QTransform::fromTranslate(pos.x(), pos.y()), ct1);
        item->toolBar()->setTransformations({ct1, ct2});
        item->setSizeMode(PageBoxItem::LargeCanvas);
    } else {
        item->setSizeMode((flags_ & FullLayout) ? PageBoxItem::FixedSize : PageBoxItem::MatchContent);
        ControlTransform * ct1 = new ControlTransform(static_cast<ControlTransform*>(transform_), true, false, false);
        QPointF pos(0, item->boundingRect().bottom() - 30);
        StaticTransform* ct2 = new StaticTransform(QTransform::fromTranslate(pos.x(), pos.y()), ct1);
        item->toolBar()->setTransformations({ct2, ct1});
    }
    item->toolBar()->hide();
    if (property("pageData").isValid()) {
        loadPages(item);
        return;
    }
    loadData();
    if (property("pageModel").isValid()) {
        loadPages(item);
        return;
    }
}

void PageBoxControl::detaching()
{
    PageBoxItem * item = static_cast<PageBoxItem *>(item_);
    item->document()->setPlugin(nullptr);
}

void PageBoxControl::resize(QSizeF const & size)
{
    PageBoxItem * item = static_cast<PageBoxItem *>(item_);
    QRectF rect(QPointF(0, 0), size);
    rect.moveCenter(QPointF(0, 0));
    item->setRect(rect);
    item->sizeChanged();
    Control::resize(size);
    if (!(res_->flags().testFlag(ResourceView::LargeCanvas)) && !transform_->children().empty()) { // maybe before attached
        QPointF pos(0, rect.bottom() - 60);
        StaticTransform* ct2 = static_cast<StaticTransform*>(transform_->children().back()->children().first());
        ct2->setTransform(QTransform::fromTranslate(pos.x(), pos.y()));
    }
}

Control::SelectMode PageBoxControl::selectTest(QPointF const & point)
{
    if (flags_ & CanSelect) {
        PageBoxItem * item = static_cast<PageBoxItem *>(item_);
        return item->selectTest(point) ? Select : NotSelect;
    } else {
        return Control::selectTest(point);
    }
}

void PageBoxControl::loadData()
{
    if (res_->url().isLocalFile()) {
        QFileInfo fi(res_->url().toLocalFile());
        if (fi.isDir()) {
            QDir dir(fi.filePath());
            QStandardItemModel * model = new QStandardItemModel();
            for (QString file : dir.entryList({"*.jpg"})) {
                QStandardItem * item = new QStandardItem;
                item->setData(QUrl::fromLocalFile(dir.filePath(file)));
                model->appendRow(item);
            }
            QPropertyBindings * bindings = new QPropertyBindings;
            bindings->addBinding("", "image");
            setProperty("pageSize", QSizeF(1656.0, 2326.0));
            setProperty("pageModel", QVariant::fromValue(model));
            setProperty("pageBindings", QVariant::fromValue(bindings));
        }
    }
}

void PageBoxControl::parseData()
{
}

void PageBoxControl::enableInkPad()
{
    PageBoxItem * item = static_cast<PageBoxItem *>(item_);
    for(QObject* c : res_->children()) {
        InkPadPlugin * ink = qobject_cast<InkPadPlugin*>(c);
        if (ink) {
            item->document()->setPlugin(ink);
            return;
        }
    }
    item->document()->setPlugin(new InkPadPlugin(res_));
}

int PageBoxControl::pageNumber()
{
    PageBoxItem * item = static_cast<PageBoxItem *>(item_);
    return item->document()->curPage();
}

void PageBoxControl::setPageNumber(int n)
{
    PageBoxItem * item = static_cast<PageBoxItem *>(item_);
    item->document()->goToPage(n);
}

QByteArray PageBoxControl::pageBoxState()
{
    PageBoxItem * item = static_cast<PageBoxItem *>(item_);
    return item->document()->saveState();
}

void PageBoxControl::setPageBoxState(QByteArray state)
{
    PageBoxItem * item = static_cast<PageBoxItem *>(item_);
    item->document()->restoreState(state);
}

void PageBoxControl::loadPages()
{
    PageBoxItem * item = static_cast<PageBoxItem *>(item_);
    loadPages(item);
}

void PageBoxControl::loadPages(PageBoxItem * item)
{
    PageBoxDocItem * doc = item->document();
    QStandardItemModel * model = nullptr;
    QPropertyBindings * bindings = nullptr;
    QSizeF size;
    QVariant pageModel = property("pageModel");
    if (!pageModel.isValid()) {
        parseData();
        pageModel = property("pageModel");
    }
    if (pageModel.isValid()) {
        model = pageModel.value<QStandardItemModel *>();
        QVariant pageSize = property("pageSize");
        size = pageSize.toSizeF();
        QVariant pageBindings = property("pageBindings");
        bindings = pageBindings.value<QPropertyBindings *>();
    }
    doc->setItemBindings(bindings);
    doc->setPageSize(size);
    doc->setItems(model);
    if (flags_ & RestoreSession) {
        item->document()->restorePosition();
    } else {
        //doc->setManualScale(
        //            item->rect().width() / item->document()->rect().width() / 1.3);
    }
    loadFinished(true);
    item->toolBar()->show();
    //*
    if (res_->flags().testFlag(ResourceView::LargeCanvas)) {
        QGraphicsItem* canvas = item_->parentItem()->parentItem();
        ResourceTransform& tr = *item->document()->detachTransform();
        ResourceTransform& tc = Control::fromItem(canvas)->resource()->transform();
        if (!qFuzzyIsNull(tc.scale().m11() - 1)) {
            // canvas transform must attached with scale 1.0
            //   or will enlarge canvas to document translate scale
            //   not known why, but i adjust it to 1.0 here
            QTransform t = tc.transform();
            tc.scaleTo(1);
            tr = ResourceTransform(tr * tc.transform() * t.inverted());
            tc.attachTransform(&tr);
            tc = ResourceTransform(t);
        } else {
            tc.attachTransform(&tr);
        }
    }
    //*/
}
