#ifndef INKSTROKE_H
#define INKSTROKE_H

#include "TeachingTools_global.h"

#include <resources/strokes.h>
#include <InkCanvas_global.h>

#include <QSharedPointer>

INKCANVAS_FORWARD_DECLARE_CLASS(StrokeCollection);
INKCANVAS_FORWARD_DECLARE_CLASS(DrawingAttributes);

class InkStrokeRenderer;

class TEACHINGTOOLS_EXPORT InkStrokes : public Strokes
{
    Q_OBJECT

    Q_PROPERTY(QSharedPointer<StrokeCollection> strokes READ strokes())

public:
    Q_INVOKABLE InkStrokes(Resource* res);

    Q_INVOKABLE InkStrokes(InkStrokes & o);

    virtual ~InkStrokes() override;

signals:
    void cloned() const;

public:
    QSharedPointer<StrokeCollection> strokes();

    QtPromise::QPromise<void> load(QSizeF const & maxSize, QSharedPointer<DrawingAttributes> attr);

    bool isClone() const
    {
        return prev_;
    }

    QSizeF size() const;

    void clear();

protected:
    virtual InkStrokes* clone() const override;

private:
    QSharedPointer<StrokeCollection> strokes_;
    InkStrokeRenderer * renderer_;
    InkStrokes * next_;
    InkStrokes * prev_;
};

#endif // INKSTROKE_H
