#ifndef CHARTCONTROL_H
#define CHARTCONTROL_H

#include <core/control.h>

#include <QAbstractBarSeries>
#include <QAbstractSeries>
#include <QXYSeries>

QT_CHARTS_USE_NAMESPACE

class ChartControl : public Control
{
    Q_OBJECT

    Q_PROPERTY(QtCharts::QAbstractSeries::SeriesType seriesType READ seriesType WRITE setSeriesType)
    Q_PROPERTY(bool isPolar READ isPolar WRITE setPolar)

public:
    typedef QAbstractSeries::SeriesType SeriesType;

public:
    Q_INVOKABLE ChartControl(ResourceView * res);

public:
    SeriesType seriesType() const { return seriesType_; }

    void setSeriesType(SeriesType type);

    bool isPolar() const { return isPolar_; }

    void setPolar(bool is);

private:
    ControlView * create(ControlView *parent) override;

    void attached() override;

    void onData(QByteArray data) override;

    void resize(const QSizeF &size) override;

    void updateToolButton(ToolButton * button) override;

private:
    void updateSerieses();

private:
    static QAbstractSeries * createSeries(SeriesType type, QAbstractSeries * prevInst, QVariant series);

    static QAbstractBarSeries * createBarSeries(SeriesType type);

    static QXYSeries * createXYSeries(SeriesType type);

private:
    bool isPolar_ = false;
    SeriesType seriesType_ = SeriesType::SeriesTypeLine;
    QVariant values_;
};

#endif // CHARTCONTROL_H
