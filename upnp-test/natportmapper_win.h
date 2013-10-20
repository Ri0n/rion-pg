#ifndef NATPORTMAPPER_WIN_H
#define NATPORTMAPPER_WIN_H

#include <objbase.h>
#include <comutil.h>
#include <QObject>
#include <QAbstractSocket>

class QHostAddress;
class QTimer;

typedef interface IUPnPNAT IUPnPNAT;
typedef interface IStaticPortMappingCollection IStaticPortMappingCollection;

class NatPortMapperPrivate : public QObject
{
    Q_OBJECT

    IUPnPNAT*                     _nat;
    IStaticPortMappingCollection* _collection;
    QTimer *_collectionInitTimer;

signals:
    void initialized();

public:
    NatPortMapperPrivate(QObject *parent);
    ~NatPortMapperPrivate();

    inline bool isReady() const { return _collection != NULL; }

    quint64 add(QAbstractSocket::SocketType socketType,
                int externalPort, int internalPort,
                const QHostAddress &ipAddress, const QString &name);

    bool remove(QAbstractSocket::SocketType socketType, int externalport );

private slots:
    bool initCollection();
};

#endif // NATPORTMAPPER_WIN_H
