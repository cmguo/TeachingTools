#include "pageboxcontrol.h"
#include "pageboxitem.h"
#include "pagenumberwidget.h"
#include "pageboxdocitem.h"
#include "qpropertybindings.h"
#include "inkpadplugin.h"

#include <core/resource.h>
#include <core/resourceview.h>
#include <core/resourcepage.h>
#include <core/resourcetransform.h>
#include <core/controltransform.h>
#include <views/whitecanvas.h>

#include <qcomponentcontainer.h>

#include <QStandardItemModel>
#include <QGraphicsScene>
#include <QUrl>
#include <QDir>

PageBoxControl::PageBoxControl(ResourceView * res, Flags flags, Flags clearFlags)
    : Control(res, flags | KeepAspectRatio, clearFlags)
    , bottomTransform_(nullptr)
{
    if (res_->flags().testFlag(ResourceView::LargeCanvas)) {
        flags_.setFlag(CanScale, false);
        flags_.setFlag(CanMove, false);
        flags_.setFlag(FullLayout);
    }
    setMinSize({450.24, 0});
}

bool PageBoxControl::next()
{
    PageBoxItem * item = static_cast<PageBoxItem *>(item_);
    return item->document()->pageNumberWidget()->gotoNext();
}

bool PageBoxControl::prev()
{
    PageBoxItem * item = static_cast<PageBoxItem *>(item_);
    return item->document()->pageNumberWidget()->gotoPrev();
}

QGraphicsItem* PageBoxControl::create(ResourceView *res)
{
    (void)res;
    PageBoxItem * item = new PageBoxItem;
    if (res_->flags().testFlag(ResourceView::LargeCanvas)) {
        item->setAcceptedMouseButtons(Qt::NoButton);
        res_->setProperty("editingMode", 1); // Ink
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
        *matrix = matrix->toTransform() * transform_;
    }
private:
    QTransform transform_;
};

void PageBoxControl::attaching()
{
    PageBoxItem * item = static_cast<PageBoxItem *>(item_);
    itemObj_ = item;
    if (flags_ & RestoreSession)
        res_->setProperty("pagesMode", QVariant()); // only apply once
}

void PageBoxControl::attached()
{
    PageBoxItem * item = static_cast<PageBoxItem *>(item_);
    if (res_->flags().testFlag(ResourceView::LargeCanvas)) {
        item->setSizeMode(PageBoxItem::LargeCanvas);
    } else {
        item->setSizeMode((flags_ & FullLayout) ? PageBoxItem::FixedSize : PageBoxItem::MatchContent);
        if (item->toolBar()->transformations().empty()) { // maybe reload
            ControlTransform * ct1 = new ControlTransform(static_cast<ControlTransform*>(transform_), true, false, false);
            item->toolBar()->setTransformations({ct1});
            QPointF pos(0, item->boundingRect().bottom() - 46);
            item->toolBar()->setPos(pos);
        }
        item->toolBar()->hide();
    }
    // ToolbarWidget has update problem, we do this later
    if (res_->flags().testFlag(ResourceView::LargeCanvas))
        attachSubProvider(item);
    PageBoxDocItem * doc = item->document();
    if (res_->flags().testFlag(ResourceView::ListOfPages)) {
        QObject::connect(doc, &PageBoxDocItem::currentPageChanged, this, [this](int page) {
            res_->page()->switchSubPage(page);
        });
        QObject::connect(doc, &PageBoxDocItem::layoutModeChanged, this, [this]() {
            res_->page()->clearSubPages();
        });
    }
    if (property("pageData").isValid()) {
        loadPages(item);
        return;
    }
    loadData();
    if (property("pageModel").isValid()) {
        loadPages(item);
        return;
    }
    // loading with default data
    QStandardItemModel * model = new QStandardItemModel(res_);
    model->appendRow(new QStandardItem);
    QPropertyBindings * bindings = new QPropertyBindings(res_);
    bindings->addBinding("", "image");
    item->setScaleMode(PageBoxItem::WholePage);
    doc->setItemBindings(bindings);
    doc->setPageSize(pageSize());
    doc->setItems(model);
    if (item->pagesMode() == PageBoxItem::Paper)
        item->stepMiddleScale();
}

void PageBoxControl::detaching()
{
    PageBoxItem * item = static_cast<PageBoxItem *>(item_);
    attachSubProvider(nullptr);
    item->setPlugin(nullptr);
    if (!res_->flags().testFlag(ResourceView::LargeCanvas)) {
        QList<QGraphicsTransform*> tfs(item->toolBar()->transformations());
        item->toolBar()->setTransformations({});
        for (QGraphicsTransform* tf : tfs) // must delete before item
            delete tf;
    }
}

void PageBoxControl::resize(QSizeF const & size)
{
    PageBoxItem * item = static_cast<PageBoxItem *>(item_);
    QRectF rect(QPointF(0, 0), size);
    rect.moveCenter(QPointF(0, 0));
    item->setRect(rect);
    item->sizeChanged();
    if (!res_->flags().testFlag(ResourceView::LargeCanvas)) {
        QPointF pos(0, item->boundingRect().bottom() - 46);
        item->toolBar()->setPos(pos);
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
            QStandardItemModel * model = new QStandardItemModel(res_);
            for (QString file : dir.entryList({"*.jpg"})) {
                QStandardItem * item = new QStandardItem;
                item->setData(QUrl::fromLocalFile(dir.filePath(file)));
                model->appendRow(item);
            }
            QPropertyBindings * bindings = new QPropertyBindings(res_);
            bindings->addBinding("", "image");
            setProperty("pageModel", QVariant::fromValue(model));
            setProperty("pageBindings", QVariant::fromValue(bindings));
        }
    }
}

void PageBoxControl::parseData()
{
}

QSizeF PageBoxControl::pageSize()
{
    return QSizeF(1656.0, 2326.0);
}

void PageBoxControl::enableInkPad()
{
    PageBoxItem * item = static_cast<PageBoxItem *>(item_);
    for(QObject* c : res_->children()) {
        InkPadPlugin * ink = qobject_cast<InkPadPlugin*>(c);
        if (ink) {
            item->setPlugin(ink);
            return;
        }
    }
    item->setPlugin(new InkPadPlugin(res_));
    if (!(flags_ & RestoreSession))
        whiteCanvas()->topControl()->setProperty("editingMode", 0);
}

void PageBoxControl::loadPages(int initialPage)
{
    PageBoxItem * item = static_cast<PageBoxItem *>(item_);
    item->document()->reset();
    item->setScaleMode(PageBoxItem::WholePage); // restore from ManualScale
    if (initialPage >= 0)
        item->document()->setInitialPage(initialPage);
    loadPages(item);
}

void PageBoxControl::loadPages(PageBoxItem * item)
{
    PageBoxDocItem * doc = item->document();
    QStandardItemModel * model = nullptr;
    QPropertyBindings * bindings = nullptr;
    QVariant pageModel = property("pageModel");
    if (!pageModel.isValid()) {
        parseData();
        pageModel = property("pageModel");
    }
    if (pageModel.isValid()) {
        model = pageModel.value<QStandardItemModel *>();
        QVariant pageBindings = property("pageBindings");
        bindings = pageBindings.value<QPropertyBindings *>();
    }
    doc->setItemBindings(bindings);
    doc->setPageSize(pageSize()); // page size may changed
    doc->setItems(model);
    if (flags_ & RestoreSession) {
        item->restorePosition();
    } else {
        //doc->setManualScale(
        //            item->rect().width() / item->document()->rect().width() / 1.3);
    }
    loadFinished(true);
    if (!res_->flags().testFlag(ResourceView::LargeCanvas))
        item->toolBar()->show();
    //*
    if (res_->flags().testFlag(ResourceView::LargeCanvas)) {
        QGraphicsItem* canvas = item_->parentItem()->parentItem();
        ResourceTransform& tr = *item->detachTransform();
        ResourceTransform& tc = Control::fromItem(canvas)->resource()->transform();
        QVariant attachedCanvasTransform = property("attachedCanvasTransform");
        QVariant attachedPageTransform = property("attachedPageTransform");
        if (attachedCanvasTransform.isValid()) {
            // must attach with original tranform, not known why?
            ResourceTransform t = tc;
            tc = ResourceTransform(attachedCanvasTransform.value<QTransform>());
            tr = ResourceTransform(attachedPageTransform.value<QTransform>());
            tc.attachTransform(&tr);
            tc = t;
        } else {
            attachedCanvasTransform.setValue(tc.transform());
            attachedPageTransform.setValue(tr.transform());
            setProperty("attachedCanvasTransform", attachedCanvasTransform);
            setProperty("attachedPageTransform", attachedPageTransform);
            tc.attachTransform(&tr);
        }
    }
    if (!(flags_ & RestoreSession)) {
        if (item->pagesMode() == PageBoxItem::Paper)
            item->stepMiddleScale();
    }
    item->buttonsChanged();
}
