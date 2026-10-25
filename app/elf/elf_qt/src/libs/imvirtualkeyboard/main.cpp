#include <qpa/qplatforminputcontextplugin_p.h>
#include "implatforminputcontext.h"
#include <QtCore/qobject.h>
#include <QtCore/QtDebug>

class IMPlugin : public QPlatformInputContextPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID QPlatformInputContextFactoryInterface_iid FILE "im.json")

public:
    IMPlatformInputContext *create(const QString&, const QStringList&) override;
};

IMPlatformInputContext *IMPlugin::create(const QString& system, const QStringList& paramList)
{
    Q_UNUSED(paramList);

    if (system.compare(system, QLatin1String("im"), Qt::CaseInsensitive) == 0) {

        return new IMPlatformInputContext;
    }
    return 0;
}

#include "main.moc"
