#include "inkstrokehelper.h"

#include <core/optiontoolbuttons.h>
#include <core/toolbutton.h>

#include <Windows/Controls/inkcanvas.h>
#include <Windows/Controls/inkevents.h>
#include <Windows/Ink/stroke.h>
#include <Windows/Ink/strokecollection.h>
#include <Windows/Ink/stylusshape.h>
#include <Windows/Input/stylusdevice.h>

#include <QFile>
#include <QDebug>
#include <QWidget>
#include <QSlider>
#include <QLabel>
#include <QBoxLayout>
#include <QPen>

#ifndef QT_DEBUG
#define STROKE_SELECT 0
#define TOUCH_ENABLE 1
#else
#define STROKE_SELECT 1
#define TOUCH_ENABLE 1
#endif

static constexpr char const * toolsStr =
        "stroke()||Checkable,UnionUpdate|:/teachingtools/icon/brush.svg,default;"
        "eraser()||Checkable,UnionUpdate|:/teachingtools/icon/eraser2.svg,default;";


static QString ReadAllText( const QString &path )
{
    QString ret;
    QFile f(path);
    if (f.open(QIODevice::ReadOnly)) {
        ret = QString::fromUtf8(f.readAll());
        f.close();
    } else {
        qDebug() << f.errorString();
    }
    return ret;
}


class PressureHelper : public QObject
{
public:
    PressureHelper(InkCanvas* ink)
        : QObject(ink)
        , ink_(ink)
    {
        ink_->AddHandler(InkCanvas::StrokeCollectedEvent, RoutedEventHandlerT<
                        PressureHelper, InkCanvasStrokeCollectedEventArgs, &PressureHelper::applyPressure>(this));
        QObject::connect(ink_, &InkCanvas::StrokesReplaced,
                         this, &PressureHelper::strokesReplaced);
   }

    ~PressureHelper()
    {
        // InkCanvas already destoryed
        //ink_->RemoveHandler(InkCanvas::StrokeCollectedEvent, RoutedEventHandlerT<
        //                PressureHelper, InkCanvasStrokeCollectedEventArgs, &PressureHelper::applyPressure>(this));
    }

private:
    void strokesReplaced(InkCanvasStrokesReplacedEventArgs &e)
    {
        (void) e;
        ink_->SetDefaultStylusPointDescription(Stylus::DefaultPointDescription());
    }

    void applyPressure(InkCanvasStrokeCollectedEventArgs &e)
    {
        QSharedPointer<Stroke> stroke = e.GetStroke();
        QSharedPointer<StylusPointCollection> stylusPoints = stroke->StylusPoints()->Clone();
        int n = 16;
        if (stylusPoints->size() > n) {
            for (int i = 1; i < n; ++i) {
                int m = stylusPoints->size() + i - n;
                StylusPoint point = (*stylusPoints)[m];
                float d = static_cast<float>(i) / static_cast<float>(n);
                point.SetPressureFactor(point.PressureFactor() * (1.0f - d * d));
                stylusPoints->SetItem(m, point);
            }
            --n;
        } else {
            n = 0;
        }
        //QUuid guid("52053C24-CBDD-4547-AAA1-DEFEBF7FD1E1");
        //stroke->AddPropertyData(guid, 2.0);
        stroke->SetStylusPoints(stylusPoints);
    }

private:
    InkCanvas * ink_;
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
    ink->SetEditingMode(InkCanvasEditingMode::None);
    StylusShape * shape = new StylusShape(StylusTip::Rectangle, eraserSize.width(), eraserSize.height(), 0);
    shape->setParent(ink);
    ink->SetEraserShape(shape);
    new PressureHelper(ink); // attached to InkCanvas
    return ink;
}

Control::SelectMode InkStrokeHelper::selectTest(InkCanvas *ink, const QPointF &pt, bool eatUnselect, bool mixSelect)
{
    if (ink->EditingMode() == InkCanvasEditingMode::None) {
#if STROKE_SELECT
        QSharedPointer<StrokeCollection> hits = ink->Strokes()->HitTest(pt);
        if (hits && !hits->empty()) {
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
            if (hits && !hits->empty()) {
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
    } else if (mixSelect && ink->EditingMode() == InkCanvasEditingMode::Ink) {
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

void InkStrokeHelper::updateToolButton(InkCanvas* ink, ToolButton *button)
{
    if (!button->name().startsWith("stroke(") && !button->name().startsWith("eraser("))
        return;
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
}

static StateColorToolButtons colorButtons(QList<QColor>({
                                              "#FFF0F0F0", "#FFFFCE2D", "#FFFF9F5E", "#FFFF6262", "#FF7A51AE",
                                              "#FF43CAFF", "#FF2FA8B3", "#FF506EB7", "#FF28417F", "#FF000000"
                                          }));
static StateWidthToolButtons widthButtons({2.0, 4.0, 8.0, 16.0});

void InkStrokeHelper::getToolButtons(InkCanvas* ink, QList<ToolButton *> &buttons, ToolButton *parent)
{
    if (parent->name() == "stroke(QString)") {
        colorButtons.getButtons(buttons, ink->DefaultDrawingAttributes()->Color());
        buttons.append(&ToolButton::LINE_SPLITTER);
        widthButtons.getButtons(buttons, ink->DefaultDrawingAttributes()->Width());
    } else if (parent->name() == "eraser(QString)") {
        QVariant eraseAllButton = ink->property("eraseAllButton");
        if (!eraseAllButton.isValid()) {
            QWidget * w = createEraserWidget();
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

QWidget *InkStrokeHelper::createEraserWidget()
{
    QWidget* pWidget = new QWidget(nullptr, Qt::FramelessWindowHint);
    pWidget->setFixedSize(180, 125);
    pWidget->setStyleSheet(ReadAllText(":/teachingtools/qss/inkeraser.qss"));

    QSlider* pSliter = new QSlider();
    pSliter->setOrientation(Qt::Horizontal);
    //设置滑动条控件的最小值
    pSliter->setMinimum(0);
    //设置滑动条控件的最大值
    pSliter->setMaximum(100);

    QLabel* pTextLabel = new QLabel;
    pTextLabel->setText("滑动清空笔迹");
    pTextLabel->setAlignment(Qt::AlignCenter);

    QVBoxLayout* mainLayout = new QVBoxLayout(pWidget);
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

    QObject::connect(pSliter, &QSlider::sliderReleased, pWidget, [pWidget, pSliter] {
        if (pSliter->sliderPosition() == pSliter->maximum()) {
            QVariant action = pWidget->property(ToolButton::ACTION_PROPERTY);
            if (action.isValid()) {
                action.value<ToolButton::action_t>()();
            }
        }
        pSliter->setSliderPosition(0);
    });
    pWidget->hide();
    return pWidget;
}
