#include "inkstrokehelper.h"

#include <core/toolbutton.h>

#include <Windows/Controls/inkcanvas.h>
#include <Windows/Controls/inkevents.h>
#include <Windows/Ink/stroke.h>
#include <Windows/Ink/strokecollection.h>
#include <Windows/Ink/stylusshape.h>

#include <QFile>
#include <QDebug>
#include <QWidget>
#include <QSlider>
#include <QLabel>
#include <QBoxLayout>
#include <QPen>

#define STROKE_SELECT 0

static constexpr char const * toolsStr =
        "stroke()||Checkable,UnionUpdate|:/teachingtools/icon/brush.png;"
        "eraser()||Checkable,UnionUpdate|:/teachingtools/icon/eraser2.png;";


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
    }

    ~PressureHelper()
    {
        // InkCanvas already destoryed
        //ink_->RemoveHandler(InkCanvas::StrokeCollectedEvent, RoutedEventHandlerT<
        //                PressureHelper, InkCanvasStrokeCollectedEventArgs, &PressureHelper::applyPressure>(this));
    }

private:
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

Control::SelectMode InkStrokeHelper::selectTest(InkCanvas *ink, const QPointF &pt, bool eatUnselect)
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
    if (!button->name.startsWith("stroke(") && !button->name.startsWith("eraser("))
        return;
    bool checked = button->name.startsWith("stroke(")
              ? ink->EditingMode() == InkCanvasEditingMode::Ink
              : (ink->EditingMode() == InkCanvasEditingMode::EraseByStroke
                 || ink->EditingMode() == InkCanvasEditingMode::EraseByPoint);
    button->flags.setFlag(ToolButton::Checked, checked);
    button->flags.setFlag(ToolButton::Popup, checked);
    button->flags.setFlag(ToolButton::OptionsGroup, checked);
    if (checked) {
        if (button->name == "stroke()")
            button->name = "stroke(QString)";
        if (button->name == "eraser()")
            button->name = "eraser(QString)";
    } else {
        if (button->name == "stroke(QString)")
            button->name = "stroke()";
        if (button->name == "eraser(QString)")
            button->name = "eraser()";
    }
}

static QList<ToolButton *> strokeButtons;

static QGraphicsItem* colorIcon(QColor color, bool selected)
{
    QGraphicsRectItem * item = new QGraphicsRectItem;
    item->setRect({1, 1, 30, 30});
    if (selected)
        item->setPen(QPen(Qt::white, 2.0));
    else
        item->setPen(QPen(QColor(color.red() / 2 + 128, // mix with white
                            color.green() / 2 + 128, color.blue() / 2 + 128), 2.0));
    item->setBrush(color);
    return item;
}

static QGraphicsItem* widthIcon(qreal width, bool selected)
{
    QPainterPath ph;
    ph.addEllipse(QRectF(1, 1, 30, 30));
    QGraphicsPathItem * border = new QGraphicsPathItem(ph);
    if (selected)
        border->setPen(QPen(Qt::blue, 2));
    else
        border->setPen(Qt::NoPen);
    border->setBrush(QBrush());
    QPainterPath ph2;
    QRectF rect(0, 0, width * 3, width * 3);
    rect.moveCenter(QPointF(16, 16));
    ph2.addEllipse(rect);
    QGraphicsPathItem * item = new QGraphicsPathItem(ph2, border);
    item->setPen(Qt::NoPen);
    item->setBrush(Qt::yellow);
    return border;
}

void InkStrokeHelper::getToolButtons(InkCanvas* ink, QList<ToolButton *> &buttons, ToolButton *parent)
{
    if (parent->name == "stroke(QString)") {
        if (strokeButtons.isEmpty()) {
            for (char const * c : {
                 "#FFF0F0F0", "#FFFFCE2D", "#FFFF9F5E", "#FFFF6262", "#FF46515F",
                 "#FF43CAFF", "#FF2FA8B3", "#FF46515F", "#FF28417F", "#FF000000"
             }) {
                QColor cl(c);
                QString name = QVariant(cl).toString();
                ToolButton::Flags flags = nullptr;
                QVariantMap icons;
                icons.insert("normal", QVariant::fromValue(colorIcon(cl, false)));
                icons.insert("+normal", QVariant::fromValue(colorIcon(cl, true)));
                ToolButton * btn = new ToolButton({name, "", flags, icons});
                strokeButtons.append(btn);
                if (strokeButtons.size() == 5)
                    strokeButtons.append(&ToolButton::LINE_BREAK);
            }
            strokeButtons.append(&ToolButton::LINE_BREAK);
            for (qreal w : {
                 1.0, 3.0, 5.0,
             }) {
                QString name = QVariant::fromValue(w).toString();
                ToolButton::Flags flags = nullptr;
                QVariantMap icons;
                icons.insert("normal", QVariant::fromValue(widthIcon(w, false)));
                icons.insert("+normal", QVariant::fromValue(widthIcon(w, true)));
                ToolButton * btn = new ToolButton({name, "", flags, icons});
                strokeButtons.append(btn);
            }
        }
        QString color = QVariant(ink->DefaultDrawingAttributes()->Color()).toString();
        QString width = QVariant(ink->DefaultDrawingAttributes()->Width()).toString();
        for (ToolButton* tb : strokeButtons) {
            tb->flags.setFlag(ToolButton::Selected, tb->name == color || tb->name == width);
        }
        buttons.append(strokeButtons);
    } else if (parent->name == "eraser(QString)") {
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
    pWidget->setFixedSize(160, 125);
    QSlider* pSliter = new QSlider();
    pSliter->setOrientation(Qt::Horizontal);
    //设置滑动条控件的最小值
    pSliter->setMinimum(0);
    //设置滑动条控件的最大值
    pSliter->setMaximum(100);

    QLabel* pTextLabel = new QLabel;
    QLabel* pTipLabel = new QLabel;
    pTipLabel->setContentsMargins(20,0,0,0);
    pTextLabel->setText("滑动清空笔迹");
    pTipLabel->setText("橡皮擦");
    pTextLabel->setAlignment(Qt::AlignCenter);
    QVBoxLayout* mainLayout = new QVBoxLayout(pWidget);
    pWidget->setStyleSheet(ReadAllText(":/teachingtools/qss/inkeraser.qss"));
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
   // mainLayout->addWidget(pTextLabel);

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
