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

    Q_INVOKABLE InkStrokes(InkStrokes const & o);

public:
    QSharedPointer<StrokeCollection> strokes();

    QtPromise::QPromise<void> load(QSizeF const & size, QSharedPointer<DrawingAttributes> attr);

private:
    QSharedPointer<StrokeCollection> strokes_;
};

#endif // INKSTROKE_H
