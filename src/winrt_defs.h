#ifndef WINRT_DEFS_H
#define WINRT_DEFS_H

#include <windows.h>
#include <roapi.h>
#include <winstring.h>

/*
 * Manual WinRT/COM interface definitions for toast notifications.
 * mingw-w64 provides the base WinRT infrastructure (HSTRING, IInspectable,
 * RoInitialize, etc.) but lacks the Windows.UI.Notifications and
 * Windows.Data.Xml.Dom interfaces.
 *
 * GUIDs are defined as plain static const structs to avoid INITGUID pulling
 * in hundreds of COMDAT sections from system headers, which overflows the
 * byte-sized section counter in different COFF loaders.
 */

/* ---- Runtime class names ---- */

static const wchar_t RuntimeClass_ToastNotificationManager[] =
    L"Windows.UI.Notifications.ToastNotificationManager";

static const wchar_t RuntimeClass_ToastNotification[] =
    L"Windows.UI.Notifications.ToastNotification";

static const wchar_t RuntimeClass_XmlDocument[] =
    L"Windows.Data.Xml.Dom.XmlDocument";

/* ---- IIDs ---- */

/* IXmlDocument: {f7f3a506-1e87-42d6-bcfb-b8c809fa5494} */
static const GUID IID_IXmlDocument =
    { 0xf7f3a506, 0x1e87, 0x42d6,
      { 0xbc, 0xfb, 0xb8, 0xc8, 0x09, 0xfa, 0x54, 0x94 } };

/* IXmlDocumentIO: {6cd0e74e-ee65-4489-9ebf-ca43e87ba637} */
static const GUID IID_IXmlDocumentIO =
    { 0x6cd0e74e, 0xee65, 0x4489,
      { 0x9e, 0xbf, 0xca, 0x43, 0xe8, 0x7b, 0xa6, 0x37 } };

/* IToastNotificationManagerStatics: {50ac103f-d235-4598-bbef-98fe4d1a3ad4} */
static const GUID IID_IToastNotificationManagerStatics =
    { 0x50ac103f, 0xd235, 0x4598,
      { 0xbb, 0xef, 0x98, 0xfe, 0x4d, 0x1a, 0x3a, 0xd4 } };

/* IToastNotificationFactory: {04124b20-82c6-4229-b109-fd9ed4662b53} */
static const GUID IID_IToastNotificationFactory =
    { 0x04124b20, 0x82c6, 0x4229,
      { 0xb1, 0x09, 0xfd, 0x9e, 0xd4, 0x66, 0x2b, 0x53 } };

/* IToastNotification: {997e2675-059e-4e60-8b06-1760917c8b80} */
static const GUID IID_IToastNotification =
    { 0x997e2675, 0x059e, 0x4e60,
      { 0x8b, 0x06, 0x17, 0x60, 0x91, 0x7c, 0x8b, 0x80 } };

/* IToastNotifier: {75927b93-03f3-41ec-91d3-6e5bac1b38e7} */
static const GUID IID_IToastNotifier =
    { 0x75927b93, 0x03f3, 0x41ec,
      { 0x91, 0xd3, 0x6e, 0x5b, 0xac, 0x1b, 0x38, 0xe7 } };

/* ---- Forward declarations ---- */

typedef interface IXmlDocument IXmlDocument;
typedef interface IXmlDocumentIO IXmlDocumentIO;
typedef interface IToastNotificationManagerStatics IToastNotificationManagerStatics;
typedef interface IToastNotificationFactory IToastNotificationFactory;
typedef interface IToastNotification IToastNotification;
typedef interface IToastNotifier IToastNotifier;

/* ---- IXmlDocument vtable ----
 * Inherits from IInspectable. We only need it as an opaque object
 * to pass to other interfaces; actual XML loading is done via IXmlDocumentIO.
 */
typedef struct IXmlDocumentVtbl {
    BEGIN_INTERFACE

    /* IUnknown */
    HRESULT (STDMETHODCALLTYPE *QueryInterface)(IXmlDocument *This, REFIID riid, void **ppvObject);
    ULONG   (STDMETHODCALLTYPE *AddRef)(IXmlDocument *This);
    ULONG   (STDMETHODCALLTYPE *Release)(IXmlDocument *This);

    /* IInspectable */
    HRESULT (STDMETHODCALLTYPE *GetIids)(IXmlDocument *This, ULONG *iidCount, IID **iids);
    HRESULT (STDMETHODCALLTYPE *GetRuntimeClassName)(IXmlDocument *This, HSTRING *className);
    HRESULT (STDMETHODCALLTYPE *GetTrustLevel)(IXmlDocument *This, TrustLevel *trustLevel);

    /* IXmlDocument methods (we don't call any directly) */

    END_INTERFACE
} IXmlDocumentVtbl;

interface IXmlDocument {
    CONST_VTBL IXmlDocumentVtbl *lpVtbl;
};

/* ---- IXmlDocumentIO vtable ----
 * Provides LoadXml method.
 */
typedef struct IXmlDocumentIOVtbl {
    BEGIN_INTERFACE

    /* IUnknown */
    HRESULT (STDMETHODCALLTYPE *QueryInterface)(IXmlDocumentIO *This, REFIID riid, void **ppvObject);
    ULONG   (STDMETHODCALLTYPE *AddRef)(IXmlDocumentIO *This);
    ULONG   (STDMETHODCALLTYPE *Release)(IXmlDocumentIO *This);

    /* IInspectable */
    HRESULT (STDMETHODCALLTYPE *GetIids)(IXmlDocumentIO *This, ULONG *iidCount, IID **iids);
    HRESULT (STDMETHODCALLTYPE *GetRuntimeClassName)(IXmlDocumentIO *This, HSTRING *className);
    HRESULT (STDMETHODCALLTYPE *GetTrustLevel)(IXmlDocumentIO *This, TrustLevel *trustLevel);

    /* IXmlDocumentIO */
    HRESULT (STDMETHODCALLTYPE *LoadXml)(IXmlDocumentIO *This, HSTRING xml);

    END_INTERFACE
} IXmlDocumentIOVtbl;

interface IXmlDocumentIO {
    CONST_VTBL IXmlDocumentIOVtbl *lpVtbl;
};

/* ---- IToastNotificationManagerStatics vtable ----
 * ABI slot order after IInspectable (6 slots):
 *   6: CreateToastNotifier (no args)
 *   7: CreateToastNotifierWithId
 *   8: GetTemplateContent
 */
typedef struct IToastNotificationManagerStaticsVtbl {
    BEGIN_INTERFACE

    /* IUnknown */
    HRESULT (STDMETHODCALLTYPE *QueryInterface)(IToastNotificationManagerStatics *This, REFIID riid, void **ppvObject);
    ULONG   (STDMETHODCALLTYPE *AddRef)(IToastNotificationManagerStatics *This);
    ULONG   (STDMETHODCALLTYPE *Release)(IToastNotificationManagerStatics *This);

    /* IInspectable */
    HRESULT (STDMETHODCALLTYPE *GetIids)(IToastNotificationManagerStatics *This, ULONG *iidCount, IID **iids);
    HRESULT (STDMETHODCALLTYPE *GetRuntimeClassName)(IToastNotificationManagerStatics *This, HSTRING *className);
    HRESULT (STDMETHODCALLTYPE *GetTrustLevel)(IToastNotificationManagerStatics *This, TrustLevel *trustLevel);

    /* IToastNotificationManagerStatics */
    HRESULT (STDMETHODCALLTYPE *CreateToastNotifier)(IToastNotificationManagerStatics *This, IToastNotifier **notifier);
    HRESULT (STDMETHODCALLTYPE *CreateToastNotifierWithId)(IToastNotificationManagerStatics *This, HSTRING applicationId, IToastNotifier **notifier);
    HRESULT (STDMETHODCALLTYPE *GetTemplateContent)(IToastNotificationManagerStatics *This, int type, IXmlDocument **content);

    END_INTERFACE
} IToastNotificationManagerStaticsVtbl;

interface IToastNotificationManagerStatics {
    CONST_VTBL IToastNotificationManagerStaticsVtbl *lpVtbl;
};

/* ---- IToastNotificationFactory vtable ----
 * Provides CreateToastNotification.
 */
typedef struct IToastNotificationFactoryVtbl {
    BEGIN_INTERFACE

    /* IUnknown */
    HRESULT (STDMETHODCALLTYPE *QueryInterface)(IToastNotificationFactory *This, REFIID riid, void **ppvObject);
    ULONG   (STDMETHODCALLTYPE *AddRef)(IToastNotificationFactory *This);
    ULONG   (STDMETHODCALLTYPE *Release)(IToastNotificationFactory *This);

    /* IInspectable */
    HRESULT (STDMETHODCALLTYPE *GetIids)(IToastNotificationFactory *This, ULONG *iidCount, IID **iids);
    HRESULT (STDMETHODCALLTYPE *GetRuntimeClassName)(IToastNotificationFactory *This, HSTRING *className);
    HRESULT (STDMETHODCALLTYPE *GetTrustLevel)(IToastNotificationFactory *This, TrustLevel *trustLevel);

    /* IToastNotificationFactory */
    HRESULT (STDMETHODCALLTYPE *CreateToastNotification)(IToastNotificationFactory *This, IXmlDocument *content, IToastNotification **value);

    END_INTERFACE
} IToastNotificationFactoryVtbl;

interface IToastNotificationFactory {
    CONST_VTBL IToastNotificationFactoryVtbl *lpVtbl;
};

/* ---- IToastNotification vtable ----
 * We don't call methods on this directly; it's just passed to Show().
 */
typedef struct IToastNotificationVtbl {
    BEGIN_INTERFACE

    /* IUnknown */
    HRESULT (STDMETHODCALLTYPE *QueryInterface)(IToastNotification *This, REFIID riid, void **ppvObject);
    ULONG   (STDMETHODCALLTYPE *AddRef)(IToastNotification *This);
    ULONG   (STDMETHODCALLTYPE *Release)(IToastNotification *This);

    /* IInspectable */
    HRESULT (STDMETHODCALLTYPE *GetIids)(IToastNotification *This, ULONG *iidCount, IID **iids);
    HRESULT (STDMETHODCALLTYPE *GetRuntimeClassName)(IToastNotification *This, HSTRING *className);
    HRESULT (STDMETHODCALLTYPE *GetTrustLevel)(IToastNotification *This, TrustLevel *trustLevel);

    END_INTERFACE
} IToastNotificationVtbl;

interface IToastNotification {
    CONST_VTBL IToastNotificationVtbl *lpVtbl;
};

/* ---- IToastNotifier vtable ----
 * Provides Show method.
 */
typedef struct IToastNotifierVtbl {
    BEGIN_INTERFACE

    /* IUnknown */
    HRESULT (STDMETHODCALLTYPE *QueryInterface)(IToastNotifier *This, REFIID riid, void **ppvObject);
    ULONG   (STDMETHODCALLTYPE *AddRef)(IToastNotifier *This);
    ULONG   (STDMETHODCALLTYPE *Release)(IToastNotifier *This);

    /* IInspectable */
    HRESULT (STDMETHODCALLTYPE *GetIids)(IToastNotifier *This, ULONG *iidCount, IID **iids);
    HRESULT (STDMETHODCALLTYPE *GetRuntimeClassName)(IToastNotifier *This, HSTRING *className);
    HRESULT (STDMETHODCALLTYPE *GetTrustLevel)(IToastNotifier *This, TrustLevel *trustLevel);

    /* IToastNotifier */
    HRESULT (STDMETHODCALLTYPE *Show)(IToastNotifier *This, IToastNotification *notification);
    HRESULT (STDMETHODCALLTYPE *Hide)(IToastNotifier *This, IToastNotification *notification);
    /* Additional methods (get_Setting, add_/remove_ events) omitted */

    END_INTERFACE
} IToastNotifierVtbl;

interface IToastNotifier {
    CONST_VTBL IToastNotifierVtbl *lpVtbl;
};

#endif /* WINRT_DEFS_H */
