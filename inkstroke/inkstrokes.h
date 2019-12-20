#ifndef INKSTROKE_H
#define INKSTROKE_H

#include "TeachingTools_global.h"

#include <resources/strokes.h>

#include <QSharedPointer>

class StrokeCollection;
class DrawingAttributes;

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

    QtPromise::QPromise<void> load(QSizeF const & size, QSharedPointer<DrawingAttributes> attr);

    bool isClone() const
    {
        return prev_;
    }

protected:
    virtual InkStrokes* clone() const override;

private:
    QSharedPointer<StrokeCollection> strokes_;
    InkStrokes * next_;
    InkStrokes * prev_;
};

#endif // INKSTROKE_H
