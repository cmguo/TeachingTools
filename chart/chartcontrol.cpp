#include "chartcontrol.h"

#include <core/optiontoolbuttons.h>
#include <core/toolbutton.h>

#include <QChart>
#include <QAreaSeries>
#include <QBarSeries>
#include <QBarSet>
#include <QBoxPlotSeries>
#include <QCandlestickSeries>
#include <QCandlestickSet>
#include <QHorizontalBarSeries>
#include <QHorizontalPercentBarSeries>
#include <QHorizontalStackedBarSeries>
#include <QJsonDocument>
#include <QLineSeries>
#include <QPercentBarSeries>
#include <QPieSeries>
#include <QScatterSeries>
#include <QSplineSeries>
#include <QStackedBarSeries>
#include <QMetaEnum>

static char const * toolstr =
        "isPolar|极坐标|NeedUpdate,Checkable|"
        "seriesType||Popup,OptionsGroup,NeedUpdate|;";

ChartControl::ChartControl(ResourceView * res)
    : Control(res, {LayoutScale})
{
    setToolsString(toolstr);
}

void ChartControl::setSeriesType(ChartControl::SeriesType type)
{
    seriesType_ = type;
    updateSerieses();
}

void ChartControl::setPolar(bool is)
{
    isPolar_ = is;
}

ControlView *ChartControl::create(ControlView *parent)
{
    (void) parent;
    return new QChart;
}

void ChartControl::attached()
{
    loadData();
}

static void fillProperties(QObject* target, QVariantMap props)
{
    auto i = props.keyValueBegin();
    while (i != props.keyValueEnd()) {
        target->setProperty(i->first.toUtf8(), i->second);
        ++i;
    }
}

void ChartControl::onData(QByteArray data)
{
    QJsonDocument doc = QJsonDocument::fromJson(data);
    values_ = doc.toVariant();
    qDebug() << values_;
    QChart * chart = static_cast<QChart*>(item_);
    if (values_.type() == QVariant::Map) {
        QVariantMap map = values_.toMap();
        fillProperties(chart, map);
    }
    updateSerieses();
    chart->resize({400, 300});
}

void ChartControl::resize(const QSizeF &size)
{
    QChart * chart = static_cast<QChart*>(item_);
    chart->resize(size);
    Control::resize(size);
}

void ChartControl::updateToolButton(ToolButton *button)
{
    if (button->name() == "isPolar") {
        button->setChecked(isPolar_);
    } else {
        Control::updateToolButton(button);
    }
}

void ChartControl::updateSerieses()
{
    QChart * chart = static_cast<QChart*>(item_);
    QVariantList serieses;
    if (values_.type() == QVariant::Map) {
        QVariantMap map = values_.toMap();
        serieses = map.value("serieses").toList();
    } else {
        serieses = values_.toList();
    }
    chart->removeAllSeries();
    QAbstractSeries * prev = nullptr;
    for (auto series : serieses) {
        QAbstractSeries * inst = createSeries(seriesType_, prev, series);
        if (inst != prev) {
            chart->addSeries(inst);
            prev = inst;
        }
    }
    chart->createDefaultAxes();
}

template <typename T>
struct CList : QList<T>
{
};

Q_DECLARE_METATYPE_TEMPLATE_1ARG(CList)

template <typename S, typename I, typename L, typename R>
static void fillSeries(S * inst, QVariant series, R (S::*append)(L))
{
    if (series.type() == QVariant::Map) {
        QVariantMap map = series.toMap();
        fillProperties(inst, map);
        (inst->*append)(map.value("items").value<CList<I>>());
    } else if (series.type() == QVariant::List) {
        (inst->*append)(series.value<CList<I>>());
    }
}

QAbstractBarSeries * ChartControl::createBarSeries(SeriesType type)
{
    switch (type) {
    case SeriesType::SeriesTypeBar:
        return new QBarSeries;
    case SeriesType::SeriesTypeStackedBar:
        return new QStackedBarSeries;
    case SeriesType::SeriesTypePercentBar:
        return new QPercentBarSeries;
    case SeriesType::SeriesTypeHorizontalBar:
        return new QHorizontalBarSeries;
    case SeriesType::SeriesTypeHorizontalStackedBar:
        return new QHorizontalStackedBarSeries;
    case SeriesType::SeriesTypeHorizontalPercentBar:
        return new QHorizontalPercentBarSeries;
    default:
        return nullptr;
    }
}

QXYSeries *ChartControl::createXYSeries(ChartControl::SeriesType type)
{
    switch (type) {
    case SeriesType::SeriesTypeLine:
        return new QLineSeries;
    case SeriesType::SeriesTypeScatter:
        return new QScatterSeries;
    case SeriesType::SeriesTypeSpline:
        return new QSplineSeries;
    default:
        return nullptr;
    }
}

QAbstractSeries *ChartControl::createSeries(SeriesType type, QAbstractSeries * prevInst, QVariant series)
{
    QAbstractSeries * inst = prevInst;
    switch (type) {
    case SeriesType::SeriesTypeLine:
    case SeriesType::SeriesTypeScatter:
    case SeriesType::SeriesTypeSpline: {
        QXYSeries * line = createXYSeries(type);
        fillSeries<QXYSeries, QPointF, QList<QPointF> const &>(line, series, &QXYSeries::append);
        inst = line;
        break;
    }
    case SeriesType::SeriesTypeArea: {
        QAreaSeries * area = qobject_cast<QAreaSeries*>(inst);
        if (area == nullptr) {
            inst = area = new QAreaSeries; // QLineSeries, QLineSeries
        }
        QLineSeries * line = new QLineSeries; // QPointF
        fillSeries<QXYSeries, QPointF, QList<QPointF> const &>(line, series, &QXYSeries::append);
        if (area->upperSeries()) {
            area->setLowerSeries(line);
        } else {
            area->setUpperSeries(line);
            area->setName(line->name());
        }
        break;
    }
    case SeriesType::SeriesTypeBar:
    case SeriesType::SeriesTypeStackedBar:
    case SeriesType::SeriesTypePercentBar:
    case SeriesType::SeriesTypeHorizontalBar:
    case SeriesType::SeriesTypeHorizontalStackedBar:
    case SeriesType::SeriesTypeHorizontalPercentBar: {
        QAbstractBarSeries * bar = qobject_cast<QAbstractBarSeries*>(inst);
        if (bar == nullptr) {
            bar = createBarSeries(type);
            inst = bar;
        }
        QBarSet * set = new QBarSet("Set " + QString::number(bar->count()));
        fillSeries<QBarSet, qreal, QList<qreal> const &>(set, series, &QBarSet::append);
        bar->append(set);
        break;
    }
    case SeriesType::SeriesTypePie: {
        QPieSeries * pie = new QPieSeries;
        fillSeries<QPieSeries, QPieSlice*, QList<QPieSlice*>>(pie, series, &QPieSeries::append);
        inst = pie;
        break;
    }
    case SeriesType::SeriesTypeBoxPlot: {
        QBoxPlotSeries * boxPlot = new QBoxPlotSeries;
        fillSeries<QBoxPlotSeries, QBoxSet*, QList<QBoxSet*>>(boxPlot, series, &QBoxPlotSeries::append);
        inst = boxPlot;
        break;
    }
    case SeriesType::SeriesTypeCandlestick: {
        QCandlestickSeries * candlestick = new QCandlestickSeries;
        fillSeries<QCandlestickSeries, QCandlestickSet*, QList<QCandlestickSet*> const &>(candlestick, series, &QCandlestickSeries::append);
        inst = candlestick;
        break;
    }
    }
    return inst;
}

namespace QtCharts {

// Q_ENUM

Q_DECL_CONSTEXPR const QMetaObject *qt_getEnumMetaObject(QAbstractSeries::SeriesType) noexcept { return &QAbstractSeries::staticMetaObject; }
Q_DECL_CONSTEXPR const char *qt_getEnumName(QAbstractSeries::SeriesType) noexcept { return "SeriesType"; }

}

struct SeriesTypeButtons : EnumToolButtons {
    SeriesTypeButtons()
        : EnumToolButtons(QMetaEnum::fromType<QAbstractSeries::SeriesType>())
    {
    }
    QString buttonTitle(const QVariant &value) override
    {
        return EnumToolButtons::buttonTitle(value).replace("SeriesType", "");
    }
};

static SeriesTypeButtons seriesTypeButtons;

REGISTER_OPTION_BUTTONS(ChartControl, seriesType, seriesTypeButtons)

template <template<typename> class TContainer, typename TClass, typename TClass2, typename TAppendRet = void>
static bool registerListContainerConverters(TAppendRet (TContainer<TClass>::*appendMethod)(const TClass &) = &TContainer<TClass>::append,
                                            void (TContainer<TClass>::*reserveMethod)(int) = &TContainer<TClass>::reserve)
{
    return QMetaType::registerConverter<TContainer<TClass>, QVariantList>([](const TContainer<TClass> &list) -> QVariantList {
        QVariantList l;
        l.reserve(list.size());
        for(const auto &v : list)
            l.append(QVariant::fromValue(TClass2(v)));
        return l;
    }) & QMetaType::registerConverter<QVariantList, TContainer<TClass>>([appendMethod, reserveMethod](const QVariantList &list) -> TContainer<TClass> {
        TContainer<TClass> l;
        if(reserveMethod)
            (l.*reserveMethod)(list.size());
        for(auto v : list) { // clazy:exclude=range-loop
            const auto vt = v.type();
            if(v.convert(qMetaTypeId<TClass2>()))
                (l.*appendMethod)(v.value<TClass2>());
            else {
                qWarning() << "Conversion to"
                           << QMetaType::typeName(qMetaTypeId<TContainer<TClass>>())
                           << "failed, could not convert element of type"
                           << QMetaType::typeName(vt);
                (l.*appendMethod)(TClass());
            }
        }
        return l;
    });
}

struct CPointF : QPointF {
    CPointF(QPointF const & p = {}) : QPointF(p) {}
};

Q_DECLARE_METATYPE(CPointF)

struct CReal {
    CReal(qreal v = 0) : v_(v) {}
    operator qreal() const { return v_; }
    qreal v_;
};

Q_DECLARE_METATYPE(CReal)

static struct ConverterRegister {
    ConverterRegister() {
        ::registerListContainerConverters<CList, QPointF, CPointF>();
        ::registerListContainerConverters<CList, qreal, CReal>();
        ::registerListContainerConverters<CList, QPieSlice*, QPieSlice*>();
        ::registerListContainerConverters<CList, QBoxSet*, QBoxSet*>();
        ::registerListContainerConverters<CList, QCandlestickSet*, QCandlestickSet*>();
        QMetaType::registerConverter<QVariantList, CPointF>([](auto f) -> CPointF {
            return QPointF{f.at(0).toDouble(), f.at(1).toDouble()};
        });
        QMetaType::registerConverter<QVariantList, CReal>([](auto f) -> CReal {
            return f.last().toDouble();
        });
        QMetaType::registerConverter<QVariantMap, CReal>([](auto f) -> CReal {
            return f.value("value").toDouble();
        });
        QMetaType::registerConverter<QVariantList, QPieSlice*>([](auto f) -> QPieSlice* {
            QPieSlice* slice = new QPieSlice;
            slice->setValue(f.last().toDouble());
            return slice;
        });
        QMetaType::registerConverter<QVariantMap, QPieSlice*>([](auto f) -> QPieSlice* {
            QPieSlice* slice = f.value("value").template value<QPieSlice*>();
            fillProperties(slice, f);
            return slice;
        });
        QMetaType::registerConverter<QVariantList, QBoxSet*>([](auto f) -> QBoxSet* {
            QBoxSet* set = new QBoxSet;
            auto values = QVariant(f).value<QList<qreal>>();
            if (values.size() < 5) {
                qreal avg = std::accumulate(values.begin(), values.end(), 0.0, std::plus<qreal>()) / values.size();
                while (values.size() < 5) {
                    values.insert(1, avg);
                }
            }
            set->append(values);
            return set;
        });
        QMetaType::registerConverter<QVariantMap, QBoxSet*>([](auto f) -> QBoxSet* {
            QBoxSet* set = f.value("value").template value<QBoxSet*>();
            fillProperties(set, f);
            return set;
        });
        QMetaType::registerConverter<QVariantList, QCandlestickSet*>([](auto f) -> QCandlestickSet* {
            QCandlestickSet* set = new QCandlestickSet;
            auto values = QVariant(f).value<QList<qreal>>();
            set->setOpen(values.first());
            set->setLow(values.size() > 2 ? values.at(1) : values.first());
            set->setHigh(values.size() > 2 ? values.at(values.size() - 2) : values.last());
            set->setClose(values.last());
            return set;
        });
        QMetaType::registerConverter<QVariantMap, QCandlestickSet*>([](auto f) -> QCandlestickSet* {
            QCandlestickSet* set = f.value("value").template value<QCandlestickSet*>();
            fillProperties(set, f);
            return set;
        });
    }
} converterRegister;
