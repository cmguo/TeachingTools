#include "inkstrokecontrol.h"
#include "inkstrokehelper.h"
#include "inkstroketools.h"

#include <views/whitecanvaswidget.h>
#include <views/whitecanvas.h>
#include <views/qsshelper.h>
#include <views/toolbarwidget.h>
#include <views/floatwidgetmanager.h>
#include <views/framewidget.h>
#include <core/control.h>
#include <core/optiontoolbuttons.h>
#include <core/resourcepackage.h>
#include <core/resourcepage.h>
#include <core/resourceview.h>
#include <core/toolbutton.h>

#include <QApplication>
#include <QUrl>
#include <QDebug>

static constexpr char const * toolstr =
        "nonStroke|选择|Checkable,NeedUpdate,OptionsGroup|:/teachingtools/icon/stroke.none.svg;"
        "stroke|画笔|Checkable,NeedUpdate,OptionsGroup|:/teachingtools/icon/stroke.stroke.svg;"
        "eraser|橡皮|Checkable,NeedUpdate|:/teachingtools/icon/stroke.eraser.svg;"
        ;

static StateColorToolButtons colorButtons(QList<QColor>({
    "#FFF0F0F0", "#FFFFCE2D", "#FFFF9F5E", "#FFFF6262", "#FF7A51AE",
    "#FF43CAFF", "#FF2FA8B3", "#FF506EB7", "#FF28417F", "#FF000000"
}));
static StateWidthToolButtons widthButtons({0, 4.0, 8.0, 12.0, 0});

static class StyleToolButtons : public OptionToolButtons {
public:
    StyleToolButtons()
        : OptionToolButtons({"stylusTip", "fitToCurve"})
    {
    }
protected:
    virtual QString buttonTitle(const QVariant &value) override
    {
        return value.toString();
    }
} styleButton;

static QssHelper QSS_PEN(":/teachingtools/qss/inktoolspen.qss");
static QssHelper QSS_ERASER(":/teachingtools/qss/inktoolseraser.qss");

static InkStrokeTools * inst = nullptr;

InkStrokeTools::InkStrokeTools(QObject* parent, WhiteCanvas *whiteCanvas)
    : ToolButtonProvider(parent)
    , inkControl_(nullptr)
    #if INKSTORKE_DRAWING_SETTING
    , drawTool_(nullptr)
    #endif
    , outerControl_(nullptr)
    , activeControl_(nullptr)
    #ifdef QT_DEBUG
    , mode_(InkCanvasEditingMode::None)
    #else
    , mode_(InkCanvasEditingMode::Ink)
    #endif
    , colorNormal_("#F0F0F0")
    , colorShow_("#FF6262")
    , colorOuter_("#FF6262")
    , inkColor_(&colorNormal_)
    , activeColor_(&colorNormal_)
    , width_(4.0)
{
    setToolsString(toolstr);
    followTrigger();
    if (whiteCanvas)
        attachToWhiteCanvas(whiteCanvas);
    inst = this;
}

InkStrokeTools::~InkStrokeTools()
{
    setOuterControl(nullptr);
}

InkStrokeTools *InkStrokeTools::instance()
{
    return inst;
}

void InkStrokeTools::attachToWhiteCanvas(WhiteCanvas *whiteCanvas)
{
    canvas_ = whiteCanvas;
#if INKSTORKE_DRAWING_SETTING
    drawTool_ = whiteCanvas->getToolControl("drawing");
#endif
    ResourcePackage * package = whiteCanvas->package();
    QObject::connect(package, &ResourcePackage::pageCreated, this, [](ResourcePage* page) {
        page->addResource(QUrl("inkstroke:"));
    });
    QObject::connect(canvas_, &WhiteCanvas::currentPageChanged, this, [this](ResourcePage* page) {
        switchPage(page);
    });
#if INKSTORKE_DRAWING_SETTING
    drawTool_->setProperty("width", width_);
    drawTool_->setProperty("color", *inkColor_);
#endif
    if (ResourcePage * page = whiteCanvas->page())
        switchPage(page);
}

void InkStrokeTools::switchPage(ResourcePage *page)
{
    // detach active first, last control has dead
    if (page == nullptr) {
        inkControl_ = nullptr;
        activeControl_ = outerControl_;
        inkColor_ = &colorNormal_;
        if (outerControl_ == nullptr)
            activeColor_ = inkColor_;
        return;
    }
    InkStrokeControl * control = qobject_cast<InkStrokeControl*>(canvas_->topControl());
    if (control == nullptr) // sometimes sub page change goes first, we can safely ignore
        return;
    if (inkControl_) {
        //inkControl_->disconnect(this); // old ink control is already destroyed
        inkControl_ = nullptr;
    }
    activeControl_ = outerControl_;
    inkColor_ = &colorNormal_;
    bool disabled = false;
    while (page->isSubPage())
        page = qobject_cast<ResourcePage*>(page->parent());
    if (page->isIndependentPage()) {
        inkColor_ = &colorShow_;
        if (!outerControl_) {
            QVariant editingMode = page->mainResource()->property("editingMode");
            if (editingMode.isValid()) {
                if (editingMode.toInt() < 0) {
                    editingMode = 0;
                    disabled = true;
                }
                setMode(editingMode.value<InkCanvasEditingMode>());
            }
        }
    }
    setDisabled(disabled);
    if (outerControl_ == nullptr) {
        activeColor_ = inkColor_;
        activeControl_ = control;
        colorButtons.updateValue(*activeColor_);
    }
    control->setEditingMode(mode_);
    control->setColor(*inkColor_);
    control->setWidth(width_);
    inkControl_ = control;
    if (outerControl_ == nullptr) {
        connect(inkControl_, &InkStrokeControl::editingModeChanged,
                this, [this](InkCanvasEditingMode mode) {
            if (outerControl_ == nullptr)
                setMode(mode);
        });
    }
}

void InkStrokeTools::setOuterControl(QObject *control, bool sync)
{
    if (sync && control) {
        SyncInkControl * syncControl = qobject_cast<SyncInkControl*>(outerControl_);
        if (syncControl && syncControl->outerControl_ == control)
            return;
        control = new SyncInkControl(this, control);
    }
    if (control == outerControl_)
        return;
    if (outerControl_) {
        outerControl_->disconnect(this);
        SyncInkControl * syncControl = qobject_cast<SyncInkControl*>(outerControl_);
        if (syncControl)
            delete syncControl;
        if (control == nullptr)
            sync = syncControl; // old is sync?
    }
    outerControl_ = control;
    if (outerControl_) {
        activeColor_ = &colorOuter_;
        activeControl_ = nullptr;
        setDisabled(false);
        QVariant editingMode = control->property("editingMode");
        setMode(editingMode.value<InkCanvasEditingMode>());
        activeControl_ = outerControl_;
        outerControl_->setProperty("editingMode", QVariant::fromValue(mode_));
        outerControl_->setProperty("color", colorOuter_);
        outerControl_->setProperty("width", width_);
        // avoid misunderstand
        if (inkControl_ && !sync)
            inkControl_->setEditingMode(InkCanvasEditingMode::None);
        colorButtons.updateValue(*activeColor_);
        connect(outerControl_, &QObject::destroyed, this, [this] {
            setOuterControl(nullptr);
        });
    } else {
        activeControl_ = inkControl_;
        activeColor_ = inkColor_;
        if (inkControl_ && !sync) {
            inkControl_->setEditingMode(mode_);
            inkControl_->setWidth(width_);
#if INKSTORKE_DRAWING_SETTING
            drawTool_->setProperty("width", width_);
#endif
        }
        colorButtons.updateValue(*activeColor_);
    }
}

void InkStrokeTools::setMode(InkCanvasEditingMode mode)
{
    if (mode == mode_)
        return;
    mode_ = mode;
    if (activeControl_)
        activeControl_->setProperty("editingMode", QVariant::fromValue(mode_));
    // cause update
    QList<ToolButton*> buttons;
    ToolButtonProvider::getToolButtons(buttons);
    buttons[0]->setChecked(mode_ == InkCanvasEditingMode::None);
    buttons[1]->setChecked(mode_ == InkCanvasEditingMode::Ink);
    buttons[2]->setChecked(mode_ == InkCanvasEditingMode::EraseByPoint);
}

void InkStrokeTools::setDisabled(bool b)
{
    if (b == disabled_)
        return;
    disabled_ = b;
    QList<ToolButton*> buttons;
    ToolButtonProvider::getToolButtons(buttons);
    buttons[1]->setDisabled(disabled_);
    buttons[2]->setDisabled(disabled_);
}

void InkStrokeTools::setEnabled(bool b)
{
    setDisabled(!b);
}

void InkStrokeTools::setColor(QColor color)
{
    *activeColor_ = color;
    if (activeControl_)
        activeControl_->setProperty("color", *activeColor_);
#if INKSTORKE_DRAWING_SETTING
    if (activeControl_ == inkControl_)
        drawTool_->setProperty("color", *activeColor_);
#endif
    colorButtons.updateValue(*activeColor_);
}

void InkStrokeTools::setWidth(qreal width)
{
    width_ = width;
    if (activeControl_)
        activeControl_->setProperty("width", width_);
#if INKSTORKE_DRAWING_SETTING
    if (activeControl_ == inkControl_)
        drawTool_->setProperty("width", width_);
#endif
    widthButtons.updateValue(width_);
}

void InkStrokeTools::clearInkStroke()
{
    if (activeControl_) {
        activeControl_->metaObject()->invokeMethod(activeControl_, "clear");
    }
    setMode(InkCanvasEditingMode::Ink);
}

bool InkStrokeTools::setOption(const QByteArray &key, QVariant value)
{
    bool result = true;
    if (key == "nonStroke") {
        if (!value.isValid()) {
            setMode(InkCanvasEditingMode::None);
        } else {
            inkControl_->metaObject()->invokeMethod(inkControl_, value.toByteArray());
        }
    } else if (key == "stroke") {
        if (!value.isValid()) {
            if (mode_ == InkCanvasEditingMode::Ink)
                togglePopupMenu(getStringButton(1));
            else
                setMode(InkCanvasEditingMode::Ink);
        } else if (value.toString().startsWith("#")) {
            value.convert(QVariant::Color);
            setColor(value.value<QColor>());
        } else {
            value.convert(QVariant::Double);
            setWidth(value.toDouble());
        }
    } else if (key == "select") {
        if (!value.isValid()) {
            if (mode_ == InkCanvasEditingMode::Select)
                togglePopupMenu(getStringButton(0));
            else
                setMode(InkCanvasEditingMode::Select);
        }
    } else if (key == "eraser") {
        if (!value.isValid()) {
            if (mode_ == InkCanvasEditingMode::EraseByPoint)
                togglePopupMenu(getStringButton(2));
            else
                setMode(InkCanvasEditingMode::EraseByPoint);
        } else {
            clearInkStroke();
        }
    } else if (key == "mode") {
        InkCanvasEditingMode mode = value.value<InkCanvasEditingMode>();
        switch (mode) {
        case InkCanvasEditingMode::None:
            setOption("nonStroke", QVariant());
            break;
        case InkCanvasEditingMode::Ink:
            setOption("stroke", QVariant());
            break;
        case InkCanvasEditingMode::Select:
            setOption("select", QVariant());
            break;
        case InkCanvasEditingMode::EraseByPoint:
            setOption("eraser", QVariant());
            break;
        default:
            setMode(mode);
            break;
        }
    } else {
        result = ToolButtonProvider::setOption(key, value);
    }
    return result;
}

void InkStrokeTools::getToolButtons(QList<ToolButton *> &buttons, ToolButton *parent)
{
    if (parent && parent->name() == "stroke") {
        colorButtons.getButtons(buttons, *activeColor_);
        buttons.append(&ToolButton::LINE_SPLITTER);
        widthButtons.getButtons(buttons, width_);
        followTrigger(buttons, parent);
    } else if (parent && parent->name() == "nonStroke") {
        styleButton.getButtons(buttons, "");
        followTrigger(buttons, parent);
    }
    ToolButtonProvider::getToolButtons(buttons, parent);
}

void InkStrokeTools::updateToolButton(ToolButton *button)
{
    if (button->name() == "nonStroke") {
        button->setChecked(mode_ == InkCanvasEditingMode::None);
    } else if (button->name() == "stroke") {
        button->setChecked(mode_ == InkCanvasEditingMode::Ink);
        colorButtons.updateValue(*activeColor_);
        widthButtons.updateValue(width_);
    } else if (button->name() == "eraser") {
        button->setChecked(mode_ == InkCanvasEditingMode::EraseByPoint);
    } else {
        ToolButtonProvider::updateToolButton(button);
    }
}

void InkStrokeTools::togglePopupMenu(ToolButton *button)
{
    QWidget* widget = button->data().value<QWidget*>();
    if (widget == nullptr) {
        widget = createWidget(button);
        button->setData(QVariant::fromValue(widget));
        if (button->associatedWidgets().isEmpty()) {
            widget->setParent(canvas_->scene()->views().first()->parentWidget());
            widget->move(200, 100);
        } else {
            QWidget* btn = button->associatedWidgets().first();
            widget->setParent(btn->window());
            FloatWidgetManager::from(btn)->addWidget(widget, button);
        }
    }
    widget->setVisible(!widget->isVisible());
}

QWidget *InkStrokeTools::createWidget(ToolButton *button)
{
    QWidget * widget = nullptr;
    if (button->name() == "stroke")
        widget = createPenWidget(button);
    else if (button->name() == "nonStroke")
        widget = createSelectWidget(button);
    else if (button->name() == "eraser")
        widget = createEraserWidget(button);
    else
        return nullptr;
    return new FrameWidget(widget);
}

QWidget *InkStrokeTools::createPenWidget(ToolButton *button)
{
    QList<ToolButton *> buttons;
    ToolButtonProvider::getToolButtons(buttons, QList<ToolButton*>({button}));
    ToolbarWidget bar;
    QWidget * widget = bar.createPopup(buttons);
    widget->setObjectName("inktoolspen");
    widget->setStyleSheet(QSS_PEN);
    return widget;
}

QWidget *InkStrokeTools::createSelectWidget(ToolButton *button)
{
    QList<ToolButton *> buttons;
    ToolButtonProvider::getToolButtons(buttons, QList<ToolButton*>({button}));
    ToolbarWidget bar;
    QWidget * widget = bar.createPopup(buttons);
    //widget->setObjectName("inktoolsselect");
    //widget->setStyleSheet(QSS_PEN);
    return widget;
}

QWidget *InkStrokeTools::createEraserWidget(ToolButton *button)
{
    QWidget * widget = InkStrokeHelper::createEraserWidget(QSS_ERASER);
    ToolButton::action_t action([this, widget, button]() {
        widget->hide();
        setOption(button->name(), "");
    });
    widget->setProperty(ToolButton::ACTION_PROPERTY, QVariant::fromValue(action));
    return widget;
}


SyncInkControl::SyncInkControl(InkStrokeTools *tools, QObject *outerControl)
    : tools_(tools)
    , outerControl_(outerControl)
{
    connect(outerControl, &QObject::destroyed, this, [this] {
        delete this;
    });
}

void SyncInkControl::setEditingMode(int mode)
{
    outerControl_->setProperty("editingMode", mode);
    tools_->inkControl_->setProperty("editingMode", mode);
}

int SyncInkControl::editingMode() const
{
    return outerControl_->property("editingMode").toInt();
}

void SyncInkControl::clear()
{
    outerControl_->metaObject()->invokeMethod(outerControl_, "clear");
    tools_->inkControl_->clear();
}

void SyncInkControl::setWidth(qreal width)
{
    outerControl_->setProperty("width", width);
    tools_->inkControl_->setWidth(width);
#if INKSTORKE_DRAWING_SETTING
    tools_->drawTool_->setProperty("width", width);
#endif
}

void SyncInkControl::setColor(const QColor &color)
{
    outerControl_->setProperty("color", color);
    *tools_->inkColor_ = color;
    tools_->inkControl_->setColor(color);
#if INKSTORKE_DRAWING_SETTING
    tools_->drawTool_->setProperty("color", color);
#endif
}
