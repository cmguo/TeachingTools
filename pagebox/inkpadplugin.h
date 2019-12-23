#ifndef INKPADPLUGIN_H
#define INKPADPLUGIN_H

#include "pageboxplugin.h"

#include <QSharedPointer>
#include <QVector>

class InkCanvas;
class StrokeCollection;
class InkCanvasStrokeCollectedEventArgs;

class InkPadPlugin : public PageBoxPlugin
{
    Q_OBJECT
public:
    InkPadPlugin(QObject * parent = nullptr);

public slots:
    void stroke();

    void eraser();

protected:
    virtual void onRelayout(int pageCount, int curPage) override;

    virtual void onPageChanged(int lastPage, int curPage) override;

    virtual void onSizeChanged(const QSizeF &docSize, const QSizeF &pageSize,
                               const QSizeF &viewSize) override;

private:
    void onStrokeCollected(InkCanvasStrokeCollectedEventArgs& e);

private:
    InkCanvas * inkCanvas_;
    QVector<QSharedPointer<StrokeCollection>> pageStrokes_;
};

#endif // INKPADPLUGIN_H
