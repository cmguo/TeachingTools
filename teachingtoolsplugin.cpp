#include "teachingtoolsplugin.h"
#include "inkstroke/inkstroketools.h"
#include "inkstroke/inkstrokehelper.h"
#include "controls.h"

static QExport<InkStrokeTools, ToolButtonProvider> export_ink_tools(QPart::shared);
static QExport<InkStrokeTools> export_ink_tools2(QPart::shared);
static QExport<InkStrokeHelper> export_ink_helper(QPart::shared);

TeachingToolsPlugin::TeachingToolsPlugin(QObject *parent)
    : QGenericPlugin(parent)
{
}

QObject *TeachingToolsPlugin::create(const QString &, const QString &)
{
    return nullptr;
}
