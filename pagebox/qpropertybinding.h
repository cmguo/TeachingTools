#ifndef QPROPERTYBINDING_H
#define QPROPERTYBINDING_H

#include <QVariant>
#include <QObject>

class QPropertyBinding : public QObject
{
    Q_OBJECT

public:
    explicit QPropertyBinding(QObject *parent = nullptr);

    QPropertyBinding(QPropertyBinding & o);

    virtual ~QPropertyBinding() override;

public:
    void setSource(QVariant const & src);

    void setTarget(QVariant const & dst);

    void setSourceProperty(char const * prop);

    void setTargetProperty(char const * prop);

    QPropertyBinding * bind(QVariant const & dst, QVariant const & src);

private:
    void connect(QVariant & obj, char const * prop);

    void disconnect(QVariant & obj);

    void push();

    void pull();

    static QVariant getProp(QVariant & obj, char const * prop);

    static void setProp(QVariant & obj, char const * prop, QVariant && value);

    static QMetaObject const * getMeta(QVariant & obj);

private slots:
    void onNotify();

private:
    QVariant src_;
    QVariant dst_;
    char const * src_prop_;
    char const * dst_prop_;
};

#endif // QPROPERTYBINDING_H
