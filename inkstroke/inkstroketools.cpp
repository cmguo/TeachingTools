#include "inkstrokecontrol.h"
#include "inkstrokehelper.h"
#include "inkstroketools.h"

#include <views/whitecanvaswidget.h>
#include <views/whitecanvas.h>
#include <views/qsshelper.h>
#include <views/toolbarwidget.h>
#include <views/floatwidgetmanager.h>
#include <core/control.h>
#include <core/optiontoolbuttons.h>
#include <core/resourcepackage.h>
#include <core/resourcepage.h>
#include <core/resourceview.h>
#include <core/toolbutton.h>

#include <QApplication>
#include <QUrl>

static constexpr char const * toolstr =
        "nonStroke|选择|Checkable,NeedUpdate|:/teachingtools/icon/stroke.none.svg;"
        "stroke|画笔|Checkable,NeedUpdate,OptionsGroup|:/teachingtools/icon/stroke.stroke.svg;"
        "eraser|橡皮|Checkable,NeedUpdate|:/teachingtools/icon/stroke.eraser.svg;"
        ;


static class MyColorToolButtons : public OptionToolButtons
{
public:
    MyColorToolButtons()
        : OptionToolButtons(QList<QColor>({
            "#1A1E28", "#F0F0F0", "#96A4B2", "#FF274B", "#FE9400",
            "#FFDF3F", "#00D97F", "#00DDDD", "#00AAFF", "#B744D4"}), 5)
    {
    }
private:
    virtual QVariant buttonIcon(const QVariant &value) override
    {
        QVariant vcolor(value);
        vcolor.convert(qMetaTypeId<QColor>());
        QColor color = vcolor.value<QColor>();
        QVariantMap icons;
        icons.insert("normal", QVariant::fromValue(colorIcon(color, false)));
        icons.insert("+normal", QVariant::fromValue(colorIcon(color, true)));
        return icons;
    }
    static QGraphicsItem* colorIcon(QColor color, bool selected)
    {
        QPainterPath ph;
        ph.addRoundedRect(QRectF(1, 1, 30, 30), 3, 3);
        QGraphicsPathItem * item = new QGraphicsPathItem(ph);
        item->setPen(Qt::NoPen);
        item->setBrush(color);
        if (selected) {
            static QPixmap pixmap(":/teachingtools/icon/colorchecked.png");
            QGraphicsPixmapItem * check = new QGraphicsPixmapItem(item);
            check->setPixmap(pixmap);
            check->setOffset(-pixmap.width() / 2, -pixmap.height() / 2);
            check->setScale(32.0 / pixmap.width());
            check->setPos(16, 16);
        }
        return item;
    }
} colorButtons;
static StateWidthToolButtons widthButtons({0, 4.0, 8.0, 12.0, 0}, "#2E4A66", "#A4C3ED");

static QssHelper QSS_PEN(":/teachingtools/qss/inktoolspen.qss");
static QssHelper QSS_ERASER(":/teachingtools/qss/inktoolseraser.qss");

InkStrokeTools::InkStrokeTools(QObject* parent, WhiteCanvas *whiteCanvas)
    : ToolButtonProvider(parent)
    , inkControl_(nullptr)
    , outerControl_(nullptr)
    , mode_(InkCanvasEditingMode::None)
    , colorNormal_("#F0F0F0")
    , colorShow_("#FF274B")
    , colorOuter_("#FF274B")
    , activeColor_(&colorNormal_)
    , activeControl_(nullptr)
    , width_(4.0)
{
    setToolsString(toolstr);
    followTrigger();
    if (whiteCanvas)
        attachToWhiteCanvas(whiteCanvas);
}

void InkStrokeTools::attachToWhiteCanvas(WhiteCanvas *whiteCanvas)
{
    canvas_ = whiteCanvas;
    ResourcePackage * package = whiteCanvas->package();
    QObject::connect(package, &ResourcePackage::pageCreated, this, [](ResourcePage* page) {
        page->addResource(QUrl("inkstroke:"));
    });
    QObject::connect(package, &ResourcePackage::currentPageChanged, this, [this](ResourcePage* page) {
        switchPage(page);
    });
    if (ResourcePage * page = whiteCanvas->page())
        switchPage(page);
}

void InkStrokeTools::switchPage(ResourcePage *page)
{
    // detach active first, last control has dead
    inkControl_ = nullptr;
    activeControl_ = outerControl_;
    InkStrokeControl * control = qobject_cast<InkStrokeControl*>(canvas_->topControl());
    QColor * color = &colorNormal_;
    if (!outerControl_ && page->isIndependentPage()) {
        color = &colorShow_;
        QVariant editingMode = page->mainResource()->property("editingMode");
        if (editingMode.isValid())
            setMode(editingMode.value<InkCanvasEditingMode>());
    }
    if (outerControl_ == nullptr) {
        activeColor_ = color;
        activeControl_ = control;
        colorButtons.updateValue(*activeColor_);
    }
    control->setEditingMode(mode_);
    control->setColor(*color);
    control->setWidth(width_);
    inkControl_ = control;
    connect(inkControl_, &InkStrokeControl::editingModeChanged,
            this, [this](InkCanvasEditingMode mode) {
        setMode(mode);
    });
}

void InkStrokeTools::setOuterControl(QObject *control)
{
    if (control == outerControl_)
        return;
    outerControl_ = control;
    if (outerControl_) {
        activeColor_ = &colorOuter_;
        activeControl_ = nullptr;
        QVariant editingMode = control->property("editingMode");
        setMode(editingMode.value<InkCanvasEditingMode>());
        activeControl_ = outerControl_;
        outerControl_->setProperty("editingMode", QVariant::fromValue(mode_));
        outerControl_->setProperty("color", colorOuter_);
        outerControl_->setProperty("width", width_);
        colorButtons.updateValue(*activeColor_);
    } else {
        activeControl_ = inkControl_;
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

void InkStrokeTools::setColor(QColor color)
{
    *activeColor_ = color;
    if (activeControl_)
        activeControl_->setProperty("color", *activeColor_);
    colorButtons.updateValue(*activeColor_);
}

void InkStrokeTools::setWidth(qreal width)
{
    width_ = width;
    if (activeControl_)
        activeControl_->setProperty("width", width_);
    widthButtons.updateValue(width_);
}

void InkStrokeTools::clearInkStroke()
{
    if (activeControl_) {
        activeControl_->metaObject()->invokeMethod(activeControl_, "clear");
    }
    setMode(InkCanvasEditingMode::Ink);
}

void InkStrokeTools::setOption(const QByteArray &key, QVariant value)
{
    if (key == "nonStroke") {
        setMode(InkCanvasEditingMode::None);
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
        case InkCanvasEditingMode::EraseByPoint:
            setOption("eraser", QVariant());
            break;
        default:
            setMode(mode);
            break;
        }
    } else {
        ToolButtonProvider::setOption(key, value);
    }
}

void InkStrokeTools::getToolButtons(QList<ToolButton *> &buttons, ToolButton *parent)
{
    if (parent && parent->name() == "stroke") {
        colorButtons.getButtons(buttons, *activeColor_);
        buttons.append(&ToolButton::LINE_SPLITTER);
        widthButtons.getButtons(buttons, width_);
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
    }
}

void InkStrokeTools::togglePopupMenu(ToolButton *button)
{
    QWidget* widget = button->data().value<QWidget*>();
    if (widget == nullptr) {
        widget = createWidget(button);
        button->setData(QVariant::fromValue(widget));
        QPoint pos(200, 100);
        if (button->associatedWidgets().isEmpty()) {
            widget->setParent(canvas_->scene()->views().first()->parentWidget());
        } else {
            QWidget* btn = button->associatedWidgets().first();
            widget->setParent(btn->window());
            pos = FloatWidgetManager::getPopupPosition(widget, button);
        }
        widget->move(pos);
    }
    widget->setVisible(!widget->isVisible());
}

bool InkStrokeTools::eventFilter(QObject * watched, QEvent *event)
{
    QWidget* widget = qobject_cast<QWidget*>(watched);
    if (event->type() == QEvent::Show) {
        widget->setFocus();
    }
    if (event->type() == QEvent::FocusOut) {
        if (QApplication::focusWidget()
                && QApplication::focusWidget()->parent() == watched) {
            widget->setFocus();
        } else {
            widget->hide();
        }
    }
    return false;
}

QWidget *InkStrokeTools::createWidget(ToolButton *button)
{
    QWidget * widget = button->name() == "stroke"
            ? createPenWidget(button)
            : createEraserWidget(button);
    widget->installEventFilter(this);
    return widget;
}

QWidget *InkStrokeTools::createPenWidget(ToolButton *button)
{
    QList<ToolButton *> buttons;
    ToolButtonProvider::getToolButtons(buttons, QList<ToolButton*>({button}));
    ToolbarWidget bar;
    QWidget * widget = bar.createPopup(buttons);
    widget->setStyleSheet(QSS_PEN);
    return widget;
}

QWidget *InkStrokeTools::createEraserWidget(ToolButton *button)
{
    QWidget * widget = InkStrokeHelper::createEraserWidget();
    widget->setStyleSheet(QSS_ERASER);
    ToolButton::action_t action([this, widget, button]() {
        widget->hide();
        setOption(button->name(), "");
    });
    widget->setProperty(ToolButton::ACTION_PROPERTY, QVariant::fromValue(action));
    return widget;
}

