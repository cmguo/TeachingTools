#include "inkstrokefilter.h"
#include "inkstrokegeometry.h"
#include "inkstrokehelper.h"

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
#include <views/qsshelper.h>
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

class StrokesRecordHelper : public QObject
{
public:
    StrokesRecordHelper(InkCanvas *ink);
private:
    void handle(RoutedEventArgs & args);
    void strokesChanged(StrokeCollectionChangedEventArgs & args);
    void strokesReplaced(InkCanvasStrokesReplacedEventArgs &e);
    void selectionChanging(InkCanvasSelectionEditingEventArgs &e);
    void selectionChanged(EventArgs &e);
private:
    InkCanvas *ink_ = nullptr;
    ResourceRecord * record_ = nullptr;
    MergeRecord * mergeRecord_ = nullptr;
    InkCanvasSelectionEditingEventArgs * change_ = nullptr;
};

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
    //设置滑动条控件的最小值
    pSliter->setMinimum(0);
    //设置滑动条控件的最大值
    pSliter->setMaximum(100);

    QLabel* pTextLabel = new QLabel;
    pTextLabel->setText("滑动清空笔迹");
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

/* StylusGuestureHelper */

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

/* ClickThroughtHelper */

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

StrokesRecordHelper::StrokesRecordHelper(InkCanvas *ink)
    : ink_(ink)
{
    ink->AddHandler(Mouse::MouseDownEvent, RoutedEventHandlerT<
                    StrokesRecordHelper, RoutedEventArgs, &StrokesRecordHelper::handle>(this));
    ink->AddHandler(Mouse::MouseUpEvent, RoutedEventHandlerT<
                    StrokesRecordHelper, RoutedEventArgs, &StrokesRecordHelper::handle>(this));
    ink->AddHandler(Stylus::StylusDownEvent, RoutedEventHandlerT<
                    StrokesRecordHelper, RoutedEventArgs, &StrokesRecordHelper::handle>(this));
    ink->AddHandler(Stylus::StylusUpEvent, RoutedEventHandlerT<
                    StrokesRecordHelper, RoutedEventArgs, &StrokesRecordHelper::handle>(this));
    QObject::connect(ink, &InkCanvas::StrokesReplaced,
                     this, &StrokesRecordHelper::strokesReplaced);
    QObject::connect(ink, &InkCanvas::SelectionMoving,
                     this, &StrokesRecordHelper::selectionChanging);
    QObject::connect(ink, &InkCanvas::SelectionResizing,
                     this, &StrokesRecordHelper::selectionChanging);
    QObject::connect(ink, &InkCanvas::SelectionMoved,
                     this, &StrokesRecordHelper::selectionChanged);
    QObject::connect(ink, &InkCanvas::SelectionResized,
                     this, &StrokesRecordHelper::selectionChanged);
    ink_->Strokes()->setProperty("StrokesRecordHelper", QVariant::fromValue<QObject*>(this));
    QObject::connect(ink_->Strokes().get(), &StrokeCollection::StrokesChanged,
                     this, &StrokesRecordHelper::strokesChanged);
}

void StrokesRecordHelper::handle(RoutedEventArgs &args)
{
    if (&args.GetRoutedEvent() == &Mouse::MouseUpEvent
            || &args.GetRoutedEvent() == &Stylus::StylusUpEvent) {
        if (record_) {
            RecordMergeScope rs(ink_);
            rs.add(record_);
        }
    } else {
        delete record_;
    }
    record_ = nullptr;
    mergeRecord_ = nullptr;
}

class StrokesChangedRecord : public ResourceRecord
{
public:
    StrokesChangedRecord(SharedPointer<StrokeCollection> strokes, StrokeCollectionChangedEventArgs &args)
        : strokes_(strokes)
        , args_(args)
    {
        setInfo("StrokesChangedRecord");
    }
public:
    virtual void redo() override
    {
        remove(args_.Index(), args_.Removed());
        insert(args_.Index(), args_.Added());
    }
    virtual void undo() override
    {
        remove(args_.Index(), args_.Added());
        insert(args_.Index(), args_.Removed());
    }
private:
    void insert(int index, SharedPointer<StrokeCollection> strokes)
    {
        for (auto s : *strokes)
            strokes_->Insert(index++, s);
    }
    void remove(int index, SharedPointer<StrokeCollection> strokes)
    {
        for (auto s : *strokes.get()) {
            assert((*strokes_)[index] == s);
            strokes_->RemoveAt(index);
        }
    }
private:
    SharedPointer<StrokeCollection> strokes_;
    StrokeCollectionChangedEventArgs args_;
};

void StrokesRecordHelper::strokesChanged(StrokeCollectionChangedEventArgs &args)
{
    RecordMergeScope rs(ink_);
    if (rs) {
        ResourceRecord * r = new StrokesChangedRecord(ink_->Strokes(), args);
        if (sender()->parent()) {
            QObject * real = sender()->parent()->property("StrokesRecordHelper")
                    .value<QObject*>();
            StrokesRecordHelper * helper = static_cast<StrokesRecordHelper*>(real);
            if (real)
                helper->record_ = ResourceRecordSet::merge(helper->record_, helper->mergeRecord_, r);
        } else {
            record_ = ResourceRecordSet::merge(record_, mergeRecord_, r);
        }
    }
}

void StrokesRecordHelper::strokesReplaced(InkCanvasStrokesReplacedEventArgs &e)
{
    if (e.PreviousStrokes())
        e.PreviousStrokes().get()->disconnect(this);
    e.NewStrokes()->setProperty("StrokesRecordHelper", QVariant::fromValue<QObject*>(this));
    QObject::connect(e.NewStrokes().get(), &StrokeCollection::StrokesChanged,
                     this, &StrokesRecordHelper::strokesChanged);
}

void StrokesRecordHelper::selectionChanging(InkCanvasSelectionEditingEventArgs &e)
{
    change_ = &e;
}

class StrokesEditRecord : public ResourceRecord
{
public:
    StrokesEditRecord(SharedPointer<StrokeCollection> strokes, InkCanvasSelectionEditingEventArgs &args)
        : strokes_(strokes)
        , args_(args)
    {
        setInfo("StrokesEditRecord");
    }
public:
    virtual void redo() override
    {
        strokes_->Transform(MapRectToRect(args_.NewRectangle(), args_.OldRectangle()), false);
    }
    virtual void undo() override
    {
        strokes_->Transform(MapRectToRect(args_.OldRectangle(), args_.NewRectangle()), false);
    }
private:
    Matrix MapRectToRect(Rect const & target, Rect const & source)
    {
        double m11 = target.Width() / source.Width();
        double dx = target.Left() - m11 * source.Left();
        double m22 = target.Height() / source.Height();
        double dy = target.Top() - m22 * source.Top();
        return Matrix(m11, 0, 0, m22, dx, dy);
    }
private:
    SharedPointer<StrokeCollection> strokes_;
    InkCanvasSelectionEditingEventArgs args_;
};

void StrokesRecordHelper::selectionChanged(EventArgs &)
{
    RecordMergeScope rs(ink_);
    if (rs)
        rs.add(new StrokesEditRecord(ink_->GetSelectedStrokes(), *change_));
    change_ = nullptr;
}

