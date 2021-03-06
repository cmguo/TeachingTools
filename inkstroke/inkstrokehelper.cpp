#include "inkstrokefilter.h"
#include "inkstrokegeometry.h"
#include "inkstrokehelper.h"
#include "strokesrecordhelper.h"

#include <core/optiontoolbuttons.h>
#include <core/resourcerecord.h>
#include <core/toolbutton.h>

#include <Windows/Controls/inkcanvas.h>
#include <Windows/Controls/inkevents.h>
#include <Windows/Ink/drawingattributes.h>
#include <Windows/Ink/stylusshape.h>
#include <Windows/Input/mousedevice.h>
#include <Windows/Input/mouseeventargs.h>
#include <Windows/Input/stylusdevice.h>
#include <Windows/Input/styluseventargs.h>
#include <widget/qsshelper.h>
#include <views/whitecanvas.h>
#include <views/itemselector.h>

#include <QFile>
#include <QDebug>
#include <QWidget>
#include <QSlider>
#include <QLabel>
#include <QBoxLayout>
#include <QPen>
#include <QApplication>
#include <QScreen>
#include <QTimer>
#include <QElapsedTimer>

#ifndef QT_DEBUG
#define STROKE_SELECT 0
#define TOUCH_ENABLE 1
#else
#define STROKE_SELECT 1
#define TOUCH_ENABLE 1
#endif

INKCANVAS_USE_NAMESPACE

static constexpr char const * toolsStr =
        "stroke()||Checkable,UnionUpdate|:/teachingtools/icon/brush.svg,default;"
        "eraser()||Checkable,UnionUpdate|:/teachingtools/icon/eraser2.svg,default;";

InkCanvas *InkStrokeHelper::createInkCanvas(QColor color, qreal lineWidth, QSizeF eraserSize)
{
    InkCanvas * ink = new InkCanvas;
    //ink->setStyleSheet("background-color:red;");
    //ink->DefaultDrawingAttributes()->SetStylusTip(StylusTip::Rectangle);
    ink->setAcceptTouchEvents(TOUCH_ENABLE);
    ink->DefaultDrawingAttributes()->SetFitToCurve(true);
    ink->DefaultDrawingAttributes()->SetColor(color);
    ink->DefaultDrawingAttributes()->SetWidth(lineWidth);
    ink->DefaultDrawingAttributes()->SetHeight(lineWidth);
    ink->SetDefaultStylusPointDescription(Stylus::DefaultPointDescription());
    ink->SetEditingMode(InkCanvasEditingMode::None);
    QSizeF stylusGroupSize(1.0 * QApplication::primaryScreen()->physicalDotsPerInchX(),
                           1.618 * QApplication::primaryScreen()->physicalDotsPerInchY());
    ink->setProperty("StylusGroupSize", stylusGroupSize);
    if (eraserSize.width() < 10) {
        eraserSize.setWidth(eraserSize.width() * QApplication::primaryScreen()->physicalDotsPerInchX());
        eraserSize.setHeight(eraserSize.height() * QApplication::primaryScreen()->physicalDotsPerInchY());
    } else {
        eraserSize = QssHelper::sizeScale(eraserSize);
    }
    StylusShape * shape = new StylusShape(StylusTip::Rectangle, eraserSize.width(), eraserSize.height(), 0);
    //shape->setParent(ink);
    ink->SetEraserShape(shape);
    new InkStrokeGeometry(ink); // attached to InkCanvas
    new StrokesRecordHelper(ink);
    return ink;
}

Control::SelectMode InkStrokeHelper::selectTest(InkCanvas *ink, const QPointF &pt, bool eatUnselect, bool mixSelect)
{
    if (ink->EditingMode() == InkCanvasEditingMode::None) {
#if STROKE_SELECT
        QSharedPointer<StrokeCollection> hits = ink->Strokes()->HitTest(pt);
        if (hits && hits->Count() != 0) {
            ink->Select(hits);
            ink->setProperty("tempSelect", true);
            return Control::NotSelect;
        }
#endif
        return Control::PassSelect;
    } else if (ink->EditingMode() == InkCanvasEditingMode::Select &&
               ink->property("tempSelect").isValid()) {
        InkCanvasSelectionHitResult result = ink->HitTestSelection(pt);
        if (result == InkCanvasSelectionHitResult::None) {
            QSharedPointer<StrokeCollection> hits = ink->Strokes()->HitTest(pt);
            if (hits && hits->Count() != 0) {
                ink->Select(hits);
                return Control::NotSelect;
            }
            ink->setProperty("tempSelect", QVariant());
            ink->SetEditingMode(InkCanvasEditingMode::None);
            return eatUnselect ? Control::NotSelect : Control::PassSelect;
        } else {
            return Control::NotSelect;
        }
#if MIX_SELECT
    } else if (mixSelect && (ink->EditingMode() == InkCanvasEditingMode::Ink
                             || ink->EditingMode() == InkCanvasEditingMode::EraseByPoint)) {
        return Control::PassSelect2;
#endif
    } else {
        return Control::NotSelect;
    }
}

QString InkStrokeHelper::toolString()
{
    return toolsStr;
}

bool InkStrokeHelper::updateToolButton(InkCanvas* ink, ToolButton *button)
{
    if (!button->name().startsWith("stroke(") && !button->name().startsWith("eraser("))
        return false;
    bool checked = button->name().startsWith("stroke(")
              ? ink->EditingMode() == InkCanvasEditingMode::Ink
              : (ink->EditingMode() == InkCanvasEditingMode::EraseByStroke
                 || ink->EditingMode() == InkCanvasEditingMode::EraseByPoint);
    button->setChecked(checked);
    button->setPopup(checked);
    button->setOptionsGroup(checked);
    if (checked) {
        if (button->name() == "stroke()")
            button->setName("stroke(QString)");
        if (button->name() == "eraser()")
            button->setName("eraser(QString)");
    } else {
        if (button->name() == "stroke(QString)")
            button->setName("stroke()");
        if (button->name() == "eraser(QString)")
            button->setName("eraser()");
    }
    return true;
}

static StateColorToolButtons colorButtons(QList<QColor>({
    "#FFF0F0F0", "#FFFFCE2D", "#FFFF9F5E", "#FFFF6262", "#FF7A51AE",
    "#FF43CAFF", "#FF2FA8B3", "#FF506EB7", "#FF28417F", "#FF000000"
}));
static StateWidthToolButtons widthButtons({2.0, 4.0, 8.0, 16.0});

static QssHelper QSS_ERASER(":/teachingtools/qss/inkeraser.qss");

void InkStrokeHelper::getToolButtons(InkCanvas* ink, QList<ToolButton *> &buttons, ToolButton *parent)
{
    if (parent->name() == "stroke(QString)") {
        colorButtons.getButtons(buttons, ink->DefaultDrawingAttributes()->Color());
        buttons.append(&ToolButton::LINE_SPLITTER);
        widthButtons.getButtons(buttons, ink->DefaultDrawingAttributes()->Width());
    } else if (parent->name() == "eraser(QString)") {
        QVariant eraseAllButton = ink->property("eraseAllButton");
        if (!eraseAllButton.isValid()) {
            QWidget * w = createEraserWidget(QSS_ERASER);
            ToolButton* b = new ToolButton({"eraseAll()", "", ToolButton::CustomWidget,
                        QVariant::fromValue(w)});
            eraseAllButton.setValue(b);
            ink->setProperty("eraseAllButton", eraseAllButton);
            QObject::connect(ink, &QObject::destroyed, [b, w]() {
                delete b;
                delete w;
            });
        }
        buttons.append(reinterpret_cast<ToolButton*>(eraseAllButton.value<ToolButton*>()));
    }
}

class ClickThroughtHelper : public QObject
{
public:
    ClickThroughtHelper(InkCanvas *ink);
private:
    void handle(RoutedEventArgs &);
private:
    InkCanvas *ink_;
    InkStrokeFilter * filter_;
};

void InkStrokeHelper::enableClickThrought(InkCanvas *ink)
{
    new ClickThroughtHelper(ink);
}

class StylusGuestureHelper : public QObject
{
public:
    StylusGuestureHelper(InkCanvas *ink);
private:
    void handle(StylusEventArgs & args);
private:
    InkCanvas *ink_ = nullptr;
    WhiteCanvas * canvas_ = nullptr;
    bool installed_ = false;
    QElapsedTimer timer_;
    bool failed_ = false;
};

void InkStrokeHelper::enableStylusGusture(InkCanvas *canvas)
{
    new StylusGuestureHelper(canvas);
    canvas->setProperty("enableshortcuts", true);
}

QWidget *InkStrokeHelper::createEraserWidget(QssHelper const & qss)
{
    QWidget* widget = new QFrame(nullptr);
    widget->setWindowFlags(Qt::FramelessWindowHint);
    widget->setObjectName("inkeraser");
    qDebug() << "createEraserWidget" << QString(qss);
    int w = qss.value("#inkeraser", "width").toInt();
    int h = qss.value("#inkeraser", "height").toInt();
    widget->setFixedSize(w, h);
    widget->setStyleSheet(qss);
    QSlider* pSliter = new QSlider();
    pSliter->setOrientation(Qt::Horizontal);
    //?????????????????????????????????
    pSliter->setMinimum(0);
    //?????????????????????????????????
    pSliter->setMaximum(100);

    QLabel* pTextLabel = new QLabel;
    pTextLabel->setText("??????????????????");
    pTextLabel->setAlignment(Qt::AlignCenter);

    QVBoxLayout* mainLayout = new QVBoxLayout(widget);
    mainLayout->setContentsMargins(0,0,0,10);
    mainLayout->addStretch();
    mainLayout->addWidget(pTextLabel);
    mainLayout->addStretch();
    QHBoxLayout* layout = new QHBoxLayout;
    layout->setMargin(0);
    layout->setSpacing(0);
    layout->addStretch();
    layout->addWidget(pSliter);
    layout->addStretch();
    mainLayout->addLayout(layout);
    mainLayout->addStretch();

    QObject::connect(pSliter, &QSlider::sliderReleased, widget, [widget, pSliter] {
        if (pSliter->sliderPosition() == pSliter->maximum()) {
            QVariant action = widget->property(ToolButton::ACTION_PROPERTY);
            if (action.isValid()) {
                action.value<ToolButton::action_t>()();
            }
        }
        pSliter->setSliderPosition(0);
    });
    return widget;
}

/*
 * StylusGuestureHelper
 *   Pass Stylus (Touch) events to WhiteCanvas's selector
 *   Let some white canvas operation take effect in InkCanvas Ink Mode
 */

StylusGuestureHelper::StylusGuestureHelper(InkCanvas *ink)
    : QObject(ink)
    , ink_(ink)
{
    ink->AddHandler(Stylus::StylusDownEvent, RoutedEventHandlerT<
                    StylusGuestureHelper, StylusEventArgs, &StylusGuestureHelper::handle>(this));
    ink->AddHandler(Stylus::StylusMoveEvent, RoutedEventHandlerT<
                    StylusGuestureHelper, StylusEventArgs, &StylusGuestureHelper::handle>(this));
    ink->AddHandler(Stylus::StylusUpEvent, RoutedEventHandlerT<
                    StylusGuestureHelper, StylusEventArgs, &StylusGuestureHelper::handle>(this));
}

/*
 * Conditions:
 *   C1. One group, two points
 *   C2. Meet C1 in 100 ms, from StylusDown
 *   C3. Leave if C1 not meet
 */
void StylusGuestureHelper::handle(StylusEventArgs &args)
{
    if (ink_->EditingMode() != InkCanvasEditingMode::Ink)
        return;
    if (!canvas_)
        canvas_ = static_cast<WhiteCanvas*>(ink_->parentItem()->parentItem());
    StylusDevice * device = qobject_cast<StylusDevice*>(args.Device());
    auto & groups = device->StylusGroups();
    if (&args.GetRoutedEvent() == &Stylus::StylusDownEvent) {
        timer_.restart();
        failed_ = false;
    }
    if (&args.GetRoutedEvent() != &Stylus::StylusUpEvent
            && !failed_ && (installed_ || timer_.elapsed() < 100)
            && groups.size() == 1 && groups.first().pointIds.size() == 2) {
        if (!installed_) {
            //qDebug() << "StylusGuestureHelper::handle install";
            ink_->installSceneEventFilter(canvas_->selector());
            QTouchEvent & o = *args.event();
            QTouchEvent e(QEvent::TouchBegin, device->device(), o.modifiers(), o.touchPointStates(), o.touchPoints());
            canvas_->selector()->sceneEventFilter(ink_, &e);
            installed_ = true;
        }
    } else {
        if (!failed_ && (groups.size() > 1
               || (groups.size() == 1 && groups.first().pointIds.size() > 2))) {
            failed_ = true;
        }
        if (installed_) {
            //qDebug() << "StylusGuestureHelper::handle uninstall";
            ink_->removeSceneEventFilter(canvas_->selector());
            QTouchEvent & o = *args.event();
            QTouchEvent e(QEvent::TouchEnd, device->device(), o.modifiers(), o.touchPointStates(), o.touchPoints());
            canvas_->selector()->sceneEventFilter(ink_, &e);
            installed_ = false;
        }
    }
}

/*
 * ClickThroughtHelper
 *
 */

ClickThroughtHelper::ClickThroughtHelper(InkCanvas *ink)
    : QObject(ink)
    , ink_(ink)
{
    filter_ = new InkStrokeFilter(ink);
    ink->AddHandler(InkCanvas::EditingModeChangedEvent, RoutedEventHandlerT<
                    ClickThroughtHelper, RoutedEventArgs, &ClickThroughtHelper::handle>(this));
}

void ClickThroughtHelper::handle(RoutedEventArgs &)
{
    if (ink_->EditingMode() == InkCanvasEditingMode::Ink) {
        // focus defeats click pass throught
        ink_->setFlag(QGraphicsItem::ItemIsFocusable, false);
        ink_->installSceneEventFilter(filter_);
    } else if (filter_->sendingEvent()) {
        ink_->setFlag(QGraphicsItem::ItemIsFocusable, true);
        QTimer::singleShot(0, this, [this]() {
            ink_->removeSceneEventFilter(filter_);
        });
    } else {
        ink_->setFlag(QGraphicsItem::ItemIsFocusable, true);
        ink_->removeSceneEventFilter(filter_);
    }
}
