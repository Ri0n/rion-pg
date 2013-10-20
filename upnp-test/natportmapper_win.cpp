#include <QHostAddress>
#include <QMessageBox>
#include <QTimer>

#include "natportmapper_win.h"

namespace {
    class BStr
    {
        BSTR _str;
    public:
        BStr(const OLECHAR *sz) { _str = SysAllocString(sz); }
        BStr(const QString &s) { _str = SysAllocString((OLECHAR*)s.utf16()); }
        BStr(const BSTR &s) : _str(s) { }
        ~BStr() { SysFreeString(_str); }
        inline const BSTR &operator*() const { return _str; }
        QString toString()
        {
            return QString::fromWCharArray(_str);
        }
    };

    static BStr PROTO_TCP = BStr(L"TCP");
    static BStr PROTO_UDP = BStr(L"UDP");

    inline void safeRelease(IUnknown *obj) { if (obj) obj->Release(); }
}

#ifdef __MINGW32__
# define __RPC__deref_out_opt
# define __RPC__in
# define __RPC__out

typedef interface IDynamicPortMappingCollection IDynamicPortMappingCollection;
typedef interface INATEventManager INATEventManager;

MIDL_INTERFACE("6F10711F-729B-41E5-93B8-F21D0F818DF1")
IStaticPortMapping : public IDispatch
{
public:
    virtual HRESULT STDMETHODCALLTYPE get_ExternalIPAddress(
        /* [retval][out] */ __RPC__deref_out_opt BSTR *pVal) = 0;

    virtual HRESULT STDMETHODCALLTYPE get_ExternalPort(
        /* [retval][out] */ __RPC__out long *pVal) = 0;

    virtual HRESULT STDMETHODCALLTYPE get_InternalPort(
        /* [retval][out] */ __RPC__out long *pVal) = 0;

    virtual HRESULT STDMETHODCALLTYPE get_Protocol(
        /* [retval][out] */ __RPC__deref_out_opt BSTR *pVal) = 0;

    virtual HRESULT STDMETHODCALLTYPE get_InternalClient(
        /* [retval][out] */ __RPC__deref_out_opt BSTR *pVal) = 0;

    virtual HRESULT STDMETHODCALLTYPE get_Enabled(
        /* [retval][out] */ __RPC__out VARIANT_BOOL *pVal) = 0;

    virtual HRESULT STDMETHODCALLTYPE get_Description(
        /* [retval][out] */ __RPC__deref_out_opt BSTR *pVal) = 0;

    virtual HRESULT STDMETHODCALLTYPE EditInternalClient(
        /* [in] */ __RPC__in BSTR bstrInternalClient) = 0;

    virtual HRESULT STDMETHODCALLTYPE Enable(
        VARIANT_BOOL vb) = 0;

    virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE EditDescription(
        /* [in] */ __RPC__in BSTR bstrDescription) = 0;

    virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE EditInternalPort(
        /* [in] */ long lInternalPort) = 0;

};


MIDL_INTERFACE("CD1F3E77-66D6-4664-82C7-36DBB641D0F1")
IStaticPortMappingCollection : public IDispatch
{
public:
    virtual /* [restricted][hidden][helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get__NewEnum(
        /* [retval][out] */ __RPC__deref_out_opt IUnknown **pVal) = 0;

    virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Item(
        /* [in] */ long lExternalPort,
        /* [in] */ __RPC__in BSTR bstrProtocol,
        /* [retval][out] */ __RPC__deref_out_opt IStaticPortMapping **ppSPM) = 0;

    virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Count(
        /* [retval][out] */ __RPC__out long *pVal) = 0;

    virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE Remove(
        /* [in] */ long lExternalPort,
        /* [in] */ __RPC__in BSTR bstrProtocol) = 0;

    virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE Add(
        /* [in] */ long lExternalPort,
        /* [in] */ __RPC__in BSTR bstrProtocol,
        /* [in] */ long lInternalPort,
        /* [in] */ __RPC__in BSTR bstrInternalClient,
        /* [in] */ VARIANT_BOOL bEnabled,
        /* [in] */ __RPC__in BSTR bstrDescription,
        /* [retval][out] */ __RPC__deref_out_opt IStaticPortMapping **ppSPM) = 0;

};

MIDL_INTERFACE("B171C812-CC76-485A-94D8-B6B3A2794E99")
IUPnPNAT : public IDispatch
{
public:
    virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_StaticPortMappingCollection(
        /* [retval][out] */ __RPC__deref_out_opt IStaticPortMappingCollection **ppSPMs) = 0;

    virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_DynamicPortMappingCollection(
        /* [retval][out] */ __RPC__deref_out_opt IDynamicPortMappingCollection **ppDPMs) = 0;

    virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_NATEventManager(
        /* [retval][out] */ __RPC__deref_out_opt INATEventManager **ppNEM) = 0;

};

class DECLSPEC_UUID("AE1E00AA-3FD5-403C-8A27-2BBDC30CD0E1")
UPnPNAT;

#if defined(__MINGW32__) || defined(__MINGW64__)
// MinGW doesn't usually have these
const CLSID CLSID_UPnPNAT = { 0xAE1E00AA, 0x3FD5, 0x403C, { 0x8A, 0x27, 0x2B, 0xBD, 0xC3, 0x0C, 0xD0, 0xE1 } };
const IID IID_IUPnPNAT = { 0xB171C812, 0xCC76, 0x485A, { 0x94, 0xD8, 0xB6, 0xB3, 0xA2, 0x79, 0x4E, 0x99 } };
#endif

#else
#include <natupnp.h>

const CLSID CLSID_UPnPNAT = __uuidof(UPnPNAT);
const IID IID_IUPnPNAT = __uuidof(IUPnPNAT);
#endif

class StaticPortMapping
{
    IStaticPortMapping *_spm;
public:
    StaticPortMapping(IStaticPortMapping *spm) :
        _spm(spm)
    {

    }

    QHostAddress externalAddress() const
    {
        BSTR str;
        if (_spm->get_ExternalIPAddress(&str) == S_OK) {
            return QHostAddress(BStr(str).toString());
        }
        return QHostAddress();
    }

    quint16 externalPort() const
    {
        long val;
        if (SUCCEEDED(_spm->get_ExternalPort(&val))) {
            return (quint16)val;
        }
        return 0;
    }

    quint16 internalPort() const
    {
        long val;
        if (SUCCEEDED(_spm->get_InternalPort(&val))) {
            return (quint16)val;
        }
        return 0;
    }

    QAbstractSocket::SocketType protocol() const
    {

    }

    virtual HRESULT STDMETHODCALLTYPE get_Protocol(
        /* [retval][out] */ __RPC__deref_out_opt BSTR *pVal) = 0;

    virtual HRESULT STDMETHODCALLTYPE get_InternalClient(
        /* [retval][out] */ __RPC__deref_out_opt BSTR *pVal) = 0;

    virtual HRESULT STDMETHODCALLTYPE get_Enabled(
        /* [retval][out] */ __RPC__out VARIANT_BOOL *pVal) = 0;

    virtual HRESULT STDMETHODCALLTYPE get_Description(
        /* [retval][out] */ __RPC__deref_out_opt BSTR *pVal) = 0;

    virtual HRESULT STDMETHODCALLTYPE EditInternalClient(
        /* [in] */ __RPC__in BSTR bstrInternalClient) = 0;

    virtual HRESULT STDMETHODCALLTYPE Enable(
        VARIANT_BOOL vb) = 0;

    virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE EditDescription(
        /* [in] */ __RPC__in BSTR bstrDescription) = 0;

    virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE EditInternalPort(
        /* [in] */ long lInternalPort) = 0;
};



NatPortMapperPrivate::NatPortMapperPrivate(QObject *parent) :
    _nat(0), _collection(0), _collectionInitTimer(new QTimer(this))
{
    _collectionInitTimer->setInterval(1500);
    _collectionInitTimer->setSingleShot(false);
    // Initialize COM itself so this thread can use it
    HRESULT result = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
    if ( FAILED( result ) ) return; // failed

    // Access the IUPnPNAT COM interface, has Windows send UPnP messages to the NAT router
    result = CoCreateInstance( CLSID_UPnPNAT, NULL, CLSCTX_ALL, IID_IUPnPNAT, ( void** )&_nat );
    if ( FAILED( result ) || ! _nat ) return; // failed

    // Get the collection of forwarded ports from it, has Windows send UPnP messages to the NAT router
    if (!initCollection()) {
        _collectionInitTimer->start();
    }
}

NatPortMapperPrivate::~NatPortMapperPrivate() {
    safeRelease(_collection);
    safeRelease(_nat);
    CoUninitialize();
}

quint64 NatPortMapperPrivate::add(QAbstractSocket::SocketType socketType,
            int externalPort, int internalPort,
            const QHostAddress &ipAddress, const QString &name)
{
    BStr proto(socketType == QAbstractSocket::TcpSocket? L"TCP": L"UDP");
    BStr addr(ipAddress.toString());
    BStr descr(name);

    IStaticPortMapping*           Mapping;

    // Have Windows send UPnP messages to the NAT router to get it to forward a port
    if ( Mapping ) { Mapping->Release(); Mapping = NULL; } // Reuse the mapping interface pointer
    HRESULT result = _collection->Add(  // Create a new port mapping, and add it to the collection
        externalPort, // The port to forward
        *proto,          // The protocol as the text "TCP" or "UDP" in a BSTR
        internalPort, // This computer's internal LAN port to forward to, like 192.168.1.100:internalport
        *addr,          // Internal IP address to forward to, like "192.168.1.100"
        true,         // True to start forwarding now
        *descr,          // Description text the router can show in its Web configuration interface
        &Mapping );   // Access to the IStaticPortMapping interface, if this works
    if ( FAILED( result ) || ! Mapping ) return FALSE;
    return TRUE;
}

// Takes a protocol 't' for TCP or 'u' for UDP, and a port being forwarded
// Talks UPnP to the router to remove the forwarding
// Returns false if there was an error
bool NatPortMapperPrivate::remove(QAbstractSocket::SocketType socketType, int externalport )
{
    BStr proto(socketType == QAbstractSocket::TcpSocket? L"TCP": L"UDP");

    // Have Windows send UPnP messages to the NAT router to get it to stop forwarding a port
    HRESULT result = _collection->Remove(  // Remove the specified port mapping from the collection
        externalport,                     // The port being forwarded
        *proto );                            // The protocol as the text "TCP" or "UDP" in a BSTR
    if ( FAILED( result ) ) return FALSE; // Returns S_OK even if there was nothing there to remove
    return TRUE;
}

bool NatPortMapperPrivate::initCollection()
{
    HRESULT result = _nat->get_StaticPortMappingCollection( &_collection );
    if ( SUCCEEDED( result ) && _collection ) { // Frequently, result is S_OK but Collection is null
        _collectionInitTimer->stop();
        emit initialized();
        return true;
    }
    return false;
}
