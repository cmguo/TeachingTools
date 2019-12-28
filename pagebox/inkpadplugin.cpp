#include "inkpadplugin.h"
#include "pageboxdocitem.h"
#include "inkstroke/inkstrokecontrol.h"

#include <core/toolbutton.h>

#include <Windows/Controls/inkcanvas.h>
#include <Windows/Controls/inkevents.h>
#include <Windows/Ink/drawingattributes.h>

#include <QGraphicsProxyWidget>
#include <QPen>

static constexpr char const * toolsStr =
        "stroke()|书写|Checkable,NeedUpdate,UnionUpdate|;"
        "eraser()|擦除|Checkable,NeedUpdate,UnionUpdate|;";

InkPadPlugin::InkPadPlugin(QObject * parent)
    : PageBoxPlugin(parent)
{
    inkCanvas_ = InkStrokeControl::createInkCanvas(4);
    //QGraphicsProxyWidget * proxy = new QGraphicsProxyWidget;
    //proxy->setWidget(inkCanvas_);
    //proxy->setAcceptTouchEvents(true);
    item_ = inkCanvas_;
    setToolsString(toolsStr);
}

void InkPadPlugin::stroke()
{
    //if (inkCanvas_->EditingMode() == InkCanvasEditingMode::Ink)
    //    inkCanvas_->SetEditingMode(InkCanvasEditingMode::None);
    //else
        inkCanvas_->SetEditingMode(InkCanvasEditingMode::Ink);
}

void InkPadPlugin::stroke(QString const & arg)
{
    QVariant v(arg);
    if (arg.startsWith("#")) {
        v.convert(QVariant::Color);
        inkCanvas_->DefaultDrawingAttributes()->SetColor(v.value<QColor>());
    } else {
        v.convert(QVariant::Double);
        inkCanvas_->DefaultDrawingAttributes()->SetWidth(v.toDouble());
        inkCanvas_->DefaultDrawingAttributes()->SetHeight(v.toDouble());
    }
}

void InkPadPlugin::eraser()
{
    if (inkCanvas_->EditingMode() == InkCanvasEditingMode::EraseByStroke)
        inkCanvas_->SetEditingMode(InkCanvasEditingMode::None);
    else
        inkCanvas_->SetEditingMode(InkCanvasEditingMode::EraseByStroke);
}

void InkPadPlugin::onRelayout(int pageCount, int curPage)
{
    pageStrokes_.clear();
    pageStrokes_.resize(pageCount);
    onPageChanged(-1, curPage);
}

void InkPadPlugin::onPageChanged(int lastPage, int curPage)
{
    (void) lastPage;
    if (document()->layoutMode() == PageBoxDocItem::Continuous)
        curPage = 0;
    QSharedPointer<StrokeCollection> & strokes = pageStrokes_[curPage];
    if (!strokes)
        strokes.reset(new StrokeCollection);
    inkCanvas_->SetStrokes(strokes);
}

void InkPadPlugin::onSizeChanged(const QSizeF &docSize, const QSizeF &pageSize, const QSizeF &viewSize)
{
    qreal sw = viewSize.width() / pageSize.width();
    qreal sh = viewSize.height() / pageSize.height();
    qreal s = qMin(sh, sw);
    item_->setScale(1 / s);
    QSizeF size(docSize.width() / pageSize.width() * viewSize.width(),
                 docSize.height() / pageSize.height() * viewSize.height());
    inkCanvas_->SetRenderSize(size);
    inkCanvas_->setPos(docSize.width() / 2, docSize.height() / 2);
}

bool InkPadPlugin::selectTest(const QPointF &pt)
{
    return InkStrokeControl::selectTest(inkCanvas_, pt) != Control::NotSelect;
}

void InkPadPlugin::updateToolButton(ToolButton *button)
{
    bool checked = button->name.startsWith("stroke(")
              ? inkCanvas_->EditingMode() == InkCanvasEditingMode::Ink
              : inkCanvas_->EditingMode() == InkCanvasEditingMode::EraseByStroke;
    button->flags.setFlag(ToolButton::Checked, checked);
    button->flags.setFlag(ToolButton::Popup, checked);
    button->flags.setFlag(ToolButton::OptionsGroup, checked);
    if (checked) {
        if (button->name == "stroke()")
            button->name = "stroke(QString)";
    } else {
        if (button->name == "stroke(QString)")
            button->name = "stroke()";
    }
}

static QList<ToolButton *> strokeButtons;

static QGraphicsItem* colorIcon(QColor color)
{
    QGraphicsRectItem * item = new QGraphicsRectItem;
    item->setRect({1, 1, 30, 30});
    item->setPen(QPen(QColor(color.red() / 2 + 128, // mix with white
                        color.green() / 2 + 128, color.blue() / 2 + 128), 2.0));
    item->setBrush(color);
    return item;
}

static QGraphicsItem* widthIcon(qreal width)
{
    QPainterPath ph;
    ph.addEllipse(QRectF(1, 1, 30, 30));
    QGraphicsPathItem * border = new QGraphicsPathItem(ph);
    border->setPen(QPen(Qt::blue, 2));
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

void InkPadPlugin::getToolButtons(QList<ToolButton *> &buttons, ToolButton *parent)
{
    if (parent == nullptr) {
        PageBoxPlugin::getToolButtons(buttons, parent);
        return;
    }
    if (parent->name == "stroke(QString)") {
        if (strokeButtons.isEmpty()) {
            for (QColor c : {
                 Qt::black, Qt::white, Qt::gray, Qt::red, Qt::darkYellow,
                 Qt::yellow, Qt::green, Qt::darkGreen, Qt::blue, Qt::darkBlue
             }) {
                QString name = QVariant::fromValue(c).toString();
                ToolButton::Flags flags = nullptr;
                QGraphicsItem * icon = colorIcon(c);
                ToolButton * btn = new ToolButton({name, "", flags,
                     QVariant::fromValue(icon)});
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
                QGraphicsItem * icon = widthIcon(w);
                ToolButton * btn = new ToolButton({name, "", flags,
                     QVariant::fromValue(icon)});
                strokeButtons.append(btn);
            }
        }
        buttons.append(strokeButtons);
    }
}

