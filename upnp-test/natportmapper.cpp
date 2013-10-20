#include <QTimer>

#include "natportmapper.h"
#ifdef Q_OS_WIN
# include "natportmapper_win.h"
#endif

NatPortMapper::NatPortMapper(QObject *parent) :
    QObject(parent)
{
    NatPortMapperPrivate *d = new NatPortMapperPrivate(this);
    if (!d->isReady()) {
        connect(d, SIGNAL(initialized()), SLOT(platformReady()));
    } else {
        QTimer::singleShot(0, this, SLOT(platformReady()));
    }
}

void NatPortMapper::platformReady()
{
    emit initialized();
}

bool NatPortMapper::isReady() const
{
    return d->isReady();
}
