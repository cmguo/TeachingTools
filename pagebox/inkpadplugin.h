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

    Q_INVOKABLE InkPadPlugin(InkPadPlugin const & o);

public slots:
    void stroke();

    void stroke(QString const & arg);

    void eraser();

    void eraser(QString const & arg);

protected:
    virtual void onRelayout(int pageCount) override;

    virtual void onPageChanged(int lastPage, int curPage) override;

    virtual void onSizeChanged(const QSizeF &docSize, const QSizeF &pageSize) override;

    virtual bool selectTest(const QPointF &pt) override;

protected:
    virtual void getToolButtons(QList<ToolButton *> & buttons,
                                ToolButton * parent) override;

    virtual void updateToolButton(ToolButton * button) override;

private:
    InkCanvas * inkCanvas_;
    QVector<QSharedPointer<StrokeCollection>> pageStrokes_;
    bool tempSelect_ = false;
};

#endif // INKPADPLUGIN_H
