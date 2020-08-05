#include "qpropertybindings.h"
#include "qpropertybinding.h"

#include <QVariant>

QPropertyBindings::QPropertyBindings(QObject *parent) : QObject(parent)
{

}

void QPropertyBindings::addBinding(char const * src_prop, char const * dst_prop)
{
    QPropertyBinding * b = new QPropertyBinding(this);
    b->setSourceProperty(src_prop);
    b->setTargetProperty(dst_prop);
    bindings_.append(b);
}

QPropertyBinding *QPropertyBindings::getBinding(const char *dst_prop)
{
    for (QPropertyBinding * b : bindings_) {
        if (b->targetProperty() == dst_prop
                || strcmp(b->targetProperty(), dst_prop) == 0)
            return b;
    }
    return nullptr;
}

void QPropertyBindings::bind(QVariant const & dst, QVariant const & src)
{
    QPropertyBindings * bindings = new QPropertyBindings(this);
    for (QPropertyBinding * b : bindings_) {
        bindings->bindings_.append(b->bind(dst, src));
    }
    if (itemBindings_[dst])
        delete itemBindings_[dst];
    itemBindings_[dst] = bindings;
}

void QPropertyBindings::unbind(QVariant const & dst)
{
    delete itemBindings_[dst];
    itemBindings_.remove(dst);
}

void QPropertyBindings::clear()
{
    for (QPropertyBindings * bindings : itemBindings_)
        delete bindings;
    itemBindings_.clear();
}


