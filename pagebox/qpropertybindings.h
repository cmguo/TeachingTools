#ifndef QPROPERTYBINDINGS_H
#define QPROPERTYBINDINGS_H

#include "TeachingTools_global.h"

#include <QObject>
#include <QList>
#include <QMap>

class QPropertyBinding;

class TEACHINGTOOLS_EXPORT QPropertyBindings : public QObject
{
    Q_OBJECT
public:
    explicit QPropertyBindings(QObject *parent = nullptr);

public:
    void addBinding(char const * src_prop, char const * dst_prop);

    void bind(QVariant const & dst, QVariant const & src);

    void unbind(QVariant const & dst);

    void clear();

private:
    QList<QPropertyBinding *> bindings_;
    QMap<QVariant, QPropertyBindings *> itemBindings_;
};

#endif // QPROPERTYBINDINGS_H
