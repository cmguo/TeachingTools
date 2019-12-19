#include "qpropertybinding.h"

#include <QMetaProperty>

QPropertyBinding::QPropertyBinding(QObject *parent)
    : QObject(parent)
    , src_prop_(nullptr)
    , dst_prop_(nullptr)
{
}

QPropertyBinding::QPropertyBinding(QPropertyBinding & o)
    : QObject(&o)
    , src_(o.src_)
    , dst_(o.dst_)
    , src_prop_(o.src_prop_)
    , dst_prop_(o.dst_prop_)
{
}

QPropertyBinding::~QPropertyBinding()
{
    disconnect(src_);
    disconnect(dst_);
}

void QPropertyBinding::setSource(QVariant const & src)
{
    if (src_ == src)
        return;
    disconnect(src_);
    src_ = src;
    connect(src_, src_prop_);
    push();
}

void QPropertyBinding::setTarget(QVariant const & dst)
{
    if (dst_ == dst)
        return;
    //disconnect(dst_);
    dst_ = dst;
    push();
}


void QPropertyBinding::setSourceProperty(char const * prop)
{
    if (src_prop_ == prop)
        return;
    disconnect(src_);
    src_prop_ = prop;
    connect(src_, src_prop_);
    push();
}

void QPropertyBinding::setTargetProperty(char const * prop)
{
    if (dst_prop_ == prop)
        return;
    //disconnect(dst_);
    dst_prop_ = prop;
        push();
}

QPropertyBinding * QPropertyBinding::bind(QVariant const & dst, QVariant const & src)
{
    QPropertyBinding * b = new QPropertyBinding(*this);
    b->setSource(src);
    b->setTarget(dst);
    return b;
}

void QPropertyBinding::onNotify()
{
    if (sender() == src_.value<QObject *>())
        push();
    else
        pull();
}

void QPropertyBinding::connect(QVariant & obj, char const * prop)
{
    QMetaObject const * meta = getMeta(obj);
    if (meta && prop) {
        int index = meta->indexOfProperty(prop);
        if (index >= 0) {
            QMetaProperty prop = meta->property(index);
            QMetaMethod sig = prop.notifySignal();
            if (sig.isValid()) {
                QObject::connect(obj.value<QObject *>(), sig,
                                 this, metaObject()->method(0));
            }
        }
    }
}

void QPropertyBinding::disconnect(QVariant & obj)
{
    if (obj.type() == QVariant::UserType) {
        if (obj.userType() == QMetaType::QObjectStar)
            obj.value<QObject *>()->disconnect(this);
    }
}

void QPropertyBinding::push()
{
    if (src_.isNull() || !src_prop_ || dst_.isNull() || !dst_prop_)
        return;
    QVariant value = getProp(src_, src_prop_);
    setProp(dst_, dst_prop_, std::move(value));
}

void QPropertyBinding::pull()
{
    if (src_.isNull() || !src_prop_ || dst_.isNull() || !dst_prop_)
        return;
    QVariant value = getProp(dst_, dst_prop_);
    setProp(src_, src_prop_, std::move(value));
}

QVariant QPropertyBinding::getProp(QVariant & obj, char const * prop)
{
    if (prop != nullptr && prop[0] == 0)
        return obj;
    QMetaObject const * meta = getMeta(obj);
    if (meta) {
        int index = meta->indexOfProperty(prop);
        if (index >= 0) {
            QMetaProperty p = meta->property(index);
            if (meta->inherits(&QObject::staticQtMetaObject))
                return p.read(obj.value<QObject *>());
            else
                return p.readOnGadget(*reinterpret_cast<void const * const *>(obj.constData()));
        }
    }
    return QVariant();
}

void QPropertyBinding::setProp(QVariant & obj, char const * prop, QVariant && value)
{
    QMetaObject const * meta = getMeta(obj);
    if (meta) {
        int index = meta->indexOfProperty(prop);
        if (index >= 0) {
            QMetaProperty p = meta->property(index);
            if (meta->inherits(&QObject::staticQtMetaObject))
                p.write(obj.value<QObject *>(), value);
            else
                p.writeOnGadget(*reinterpret_cast<void * const *>(obj.data()), value);
        }
    }
}

QMetaObject const * QPropertyBinding::getMeta(QVariant & obj)
{
    if (obj.type() == QVariant::UserType) {
        if (obj.userType() == QMetaType::QObjectStar)
            return obj.value<QObject *>()->metaObject();
        else {
            const auto flags = QMetaType::typeFlags(obj.userType());
            if (flags.testFlag(QMetaType::PointerToGadget))
                return QMetaType::metaObjectForType(obj.userType());
        }
    }
    return nullptr;
}
