#ifndef NATPORTMAPPER_H
#define NATPORTMAPPER_H

#include <QObject>

class NatPortMapperPrivate;

class NatPortMapper : public QObject
{
    Q_OBJECT
public:
    explicit NatPortMapper(QObject *parent = 0);
    bool isReady() const;
signals:
    void initialized();
    
public slots:
private slots:
    void platformReady();
private:
    friend class NatPortMapperPrivate;
    NatPortMapperPrivate *d;
};

#endif // NATPORTMAPPER_H
