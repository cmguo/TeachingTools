#ifndef TEACHINGTOOLSPLUGIN_H
#define TEACHINGTOOLSPLUGIN_H

#include <qcomponentfactoryinterface.h>

#include <QGenericPlugin>

class TeachingToolsPlugin : public QGenericPlugin
        , public QComponentFactoryInterface
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID ComponentFactory_iid FILE "TeachingTools.json")
    Q_INTERFACES(QComponentFactoryInterface)

public:
    explicit TeachingToolsPlugin(QObject *parent = nullptr);

private:
    QObject *create(const QString &name, const QString &spec) override;
};

#endif // TEACHINGTOOLSPLUGIN_H
