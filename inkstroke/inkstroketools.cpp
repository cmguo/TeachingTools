#include "inkstrokehelper.h"
#include "inkstroketools.h"

#include <views/whitecanvaswidget.h>
#include <views/whitecanvas.h>
#include <core/control.h>
#include <core/resourcepackage.h>
#include <core/resourcepage.h>
#include <core/toolbutton.h>

#include <QUrl>

static constexpr char const * toolstr =
        "nonStroke||:/teachingtools/icon/stroke.none.png;"
        "stroke|画笔|:/teachingtools/icon/stroke.stroke.png;"
        "eraser|橡皮|:/teachingtools/icon/stroke.eraser.png;"
        ;


InkStrokeTools::InkStrokeTools()
    : activeColor_(&colorNormal_)
{
    setToolsString(toolstr);
}

void InkStrokeTools::attachToWhiteCanvas(WhiteCanvas *whiteCanvas)
{
    ResourcePackage * package = whiteCanvas->package();
    QObject::connect(package, &ResourcePackage::pageCreated, this, [](ResourcePage* page) {
        page->addResource(QUrl("inkstroke:"));
    });
    QObject::connect(package, &ResourcePackage::currentPageChanged, this, [this](ResourcePage* page) {
        Control * control = canvas_->topControl();
        control->setProperty("editingMode", QVariant::fromValue(mode_));
        QColor * color = page->resources().first() ? &colorNormal_ : &colorShow_;
        if (outerControl_ == nullptr)
            activeColor_ = color;
        control->setProperty("color", *color);
        control->setProperty("width", width_);
    });
    QObject::connect(canvas_, &WhiteCanvas::loadingChanged, this, [this](bool loading) {
        if (loading)
            mode_ = InkCanvasEditingMode::Ink;
    }, Qt::QueuedConnection);
}

void InkStrokeTools::setOuterControl(Control *control)
{
    if (control == outerControl_)
        return;
    outerControl_ = control;
    if (outerControl_) {
        activeColor_ = &colorOuter_;
        outerControl_->setProperty("color", colorOuter_);
        outerControl_->setProperty("width", width_);
    }
}

void InkStrokeTools::setColor(QColor color)
{
    *activeColor_ = color;
    Control * control = outerControl_ ? outerControl_ : canvas_->topControl();
    control->setProperty("color", *activeColor_);
}

void InkStrokeTools::setWidth(qreal width)
{
    width_ = width;
    Control * control = outerControl_ ? outerControl_ : canvas_->topControl();
    control->setProperty("width", width_);
}

void InkStrokeTools::handleToolButton(ToolButton *button, const QStringList &)
{
    bool checked = button->isChecked();
    button->setChecked(true);
    if (button->name() == "nostroke") {
        togglePopupMenu(button);
        if (checked) {
            return;
        }
        mode_ = InkCanvasEditingMode::None;
    } else if (button->name() == "stroke") {
        mode_ = InkCanvasEditingMode::Ink;
    } else if (button->name() == "eraser") {
        if (checked) {
            // show eraser
            return;
        }
        mode_ = InkCanvasEditingMode::EraseByPoint;
    }
    Control * control = outerControl_ ? outerControl_ : canvas_->topControl();
    control->setProperty("editingMode", QVariant::fromValue(mode_));
}

void InkStrokeTools::togglePopupMenu(ToolButton *button)
{
    QWidget* widget = button->data().value<QWidget*>();
    if (widget == nullptr) {
        QWidget* btn = button->associatedWidgets().first();
        widget = button->name() == "stroke"
                ? InkStrokeHelper::createPenWidget()
                : InkStrokeHelper::createEraserWidget();
        widget->setParent(button->associatedWidgets().first()->window());
        button->setData(QVariant::fromValue(widget));
        QPoint pos = btn->mapTo(widget->parentWidget(), QPoint());
        pos -= QPoint(0, widget->sizeHint().height());
        widget->move(pos);
    }
    widget->setVisible(!widget->isVisible());
}


