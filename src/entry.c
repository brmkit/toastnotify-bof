#include "beacon.h"
#include "winrt_defs.h"

#include <windows.h>
#include <winstring.h>
#include <roapi.h>

#define intFree(addr) KERNEL32$HeapFree(KERNEL32$GetProcessHeap(), 0, addr)
#define intAlloc(size) KERNEL32$HeapAlloc(KERNEL32$GetProcessHeap(), HEAP_ZERO_MEMORY, size)

WINBASEAPI void* WINAPI KERNEL32$HeapAlloc(HANDLE hHeap, DWORD dwFlags, SIZE_T dwBytes);
WINBASEAPI BOOL WINAPI KERNEL32$HeapFree(HANDLE, DWORD, PVOID);
WINBASEAPI HANDLE WINAPI KERNEL32$GetProcessHeap();
WINBASEAPI size_t __cdecl MSVCRT$wcslen(const wchar_t *_Str);
WINBASEAPI int __cdecl MSVCRT$_snwprintf(wchar_t * __restrict__ _Dest, size_t _Count, const wchar_t * __restrict__ _Format, ...);
WINBASEAPI int __cdecl MSVCRT$strcmp(const char *_Str1,const char *_Str2);

/* Base64 */
DECLSPEC_IMPORT BOOL    WINAPI CRYPT32$CryptStringToBinaryA(LPCSTR, DWORD, DWORD, BYTE *, DWORD *, DWORD *, DWORD *);
DECLSPEC_IMPORT WINBASEAPI DWORD WINAPI KERNEL32$GetLastError();

/* Registry */
DECLSPEC_IMPORT LONG WINAPI ADVAPI32$RegOpenKeyExW(HKEY hKey, LPCWSTR lpSubKey, DWORD ulOptions, DWORD samDesired, HKEY *phkResult);
DECLSPEC_IMPORT LONG WINAPI ADVAPI32$RegEnumKeyExW(HKEY hKey, DWORD dwIndex, LPWSTR lpName, LPDWORD lpcchName, LPDWORD lpReserved, LPWSTR lpClass, LPDWORD lpcchClass, PFILETIME lpftLastWriteTime);
DECLSPEC_IMPORT LONG WINAPI ADVAPI32$RegEnumValueW(HKEY hKey, DWORD dwIndex, LPWSTR lpValueName, LPDWORD lpcchValueName, LPDWORD lpReserved, LPDWORD lpType, LPBYTE lpData, LPDWORD lpcbData);
DECLSPEC_IMPORT LONG WINAPI ADVAPI32$RegCloseKey(HKEY hKey);

/* HSTRING */
DECLSPEC_IMPORT HRESULT WINAPI COMBASE$WindowsCreateString(PCNZWCH sourceString, UINT32 length, HSTRING *string);
DECLSPEC_IMPORT HRESULT WINAPI COMBASE$WindowsDeleteString(HSTRING string);

/* WinRT initialization */
DECLSPEC_IMPORT HRESULT WINAPI COMBASE$RoInitialize(RO_INIT_TYPE initType);
DECLSPEC_IMPORT void WINAPI COMBASE$RoUninitialize(void);

/* Activation */
DECLSPEC_IMPORT HRESULT WINAPI COMBASE$RoGetActivationFactory(HSTRING activatableClassId, REFIID iid, void **factory);
DECLSPEC_IMPORT HRESULT WINAPI COMBASE$RoActivateInstance(HSTRING activatableClassId, IInspectable **instance);


/* Enumerate subkeys — each subkey name is an AUMID */
static void enumSubkeys(HKEY root, const wchar_t *path)
{
    HKEY hKey;
    if (ADVAPI32$RegOpenKeyExW(root, path, 0, KEY_READ, &hKey) != ERROR_SUCCESS)
        return;

    formatp buffer;
    BeaconFormatAlloc(&buffer, 16 * 1024);

    wchar_t name[256];
    DWORD   nameLen;
    DWORD   index = 0;
    LONG    res;

    while (1) {
        nameLen = 256;
        res = ADVAPI32$RegEnumKeyExW(hKey, index, name, &nameLen,
                                      NULL, NULL, NULL, NULL);
        if (res == ERROR_NO_MORE_ITEMS)
            break;

        if (res == ERROR_SUCCESS)
            BeaconFormatPrintf(&buffer, "  %ls\n", name);

        index++;
    }

    ADVAPI32$RegCloseKey(hKey);

    BeaconPrintf(CALLBACK_OUTPUT, "%s", BeaconFormatToString(&buffer, NULL));
    BeaconFormatFree(&buffer);
}

int getAUMID(void)
{
    BeaconPrintf(CALLBACK_OUTPUT, "[Notifications\\Settings - HKCU]\n");
    enumSubkeys(HKEY_CURRENT_USER,
        L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Notifications\\Settings");

    BeaconPrintf(CALLBACK_OUTPUT, "[Notifications\\Settings - HKLM]\n");
    enumSubkeys(HKEY_LOCAL_MACHINE,
        L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Notifications\\Settings");

    return 0;
}

static HSTRING make_hstring(const wchar_t *str)
{
    HSTRING hs;
    HRESULT hr = COMBASE$WindowsCreateString(str, (UINT32)MSVCRT$wcslen(str), &hs);
    return SUCCEEDED(hr) ? hs : NULL;
}

/* Core — accetta aumid e XML già pronto in wchar_t */
static int sendToastXml(const wchar_t *aumid, const wchar_t *xmlWide)
{
    HRESULT hr;
    BOOL roInitCalled = FALSE;

    hr = COMBASE$RoInitialize(RO_INIT_MULTITHREADED);
    if (SUCCEEDED(hr)) {
        roInitCalled = TRUE;
    } else if (hr != RPC_E_CHANGED_MODE) {
        BeaconPrintf(CALLBACK_ERROR, "RoInitialize failed: 0x%08lx\n", (unsigned long)hr);
        return 1;
    }

    IToastNotificationManagerStatics *toastManager = NULL;
    HSTRING classId = make_hstring(RuntimeClass_ToastNotificationManager);
    if (!classId) goto fail_early;
    hr = COMBASE$RoGetActivationFactory(classId, &IID_IToastNotificationManagerStatics, (void **)&toastManager);
    COMBASE$WindowsDeleteString(classId);
    if (FAILED(hr)) {
        BeaconPrintf(CALLBACK_ERROR, "Failed to get ToastNotificationManager: 0x%08lx\n", (unsigned long)hr);
        goto fail_early;
    }

    IXmlDocument *xmlDoc = NULL;
    HSTRING xmlClassId = make_hstring(RuntimeClass_XmlDocument);
    if (!xmlClassId) goto fail_manager;
    hr = COMBASE$RoActivateInstance(xmlClassId, (IInspectable **)&xmlDoc);
    COMBASE$WindowsDeleteString(xmlClassId);
    if (FAILED(hr)) {
        BeaconPrintf(CALLBACK_ERROR, "Failed to create XmlDocument: 0x%08lx\n", (unsigned long)hr);
        goto fail_manager;
    }

    IXmlDocumentIO *xmlIO = NULL;
    hr = xmlDoc->lpVtbl->QueryInterface(xmlDoc, &IID_IXmlDocumentIO, (void **)&xmlIO);
    if (FAILED(hr)) {
        BeaconPrintf(CALLBACK_ERROR, "Failed to get IXmlDocumentIO: 0x%08lx\n", (unsigned long)hr);
        goto fail_xmldoc;
    }

    HSTRING xmlString = make_hstring(xmlWide);
    if (!xmlString) goto fail_xmlio;
    hr = xmlIO->lpVtbl->LoadXml(xmlIO, xmlString);
    COMBASE$WindowsDeleteString(xmlString);
    xmlIO->lpVtbl->Release(xmlIO);
    if (FAILED(hr)) {
        BeaconPrintf(CALLBACK_ERROR, "LoadXml failed: 0x%08lx\n", (unsigned long)hr);
        goto fail_xmldoc;
    }

    IToastNotificationFactory *toastFactory = NULL;
    HSTRING toastClassId = make_hstring(RuntimeClass_ToastNotification);
    if (!toastClassId) goto fail_xmldoc;
    hr = COMBASE$RoGetActivationFactory(toastClassId, &IID_IToastNotificationFactory, (void **)&toastFactory);
    COMBASE$WindowsDeleteString(toastClassId);
    if (FAILED(hr)) {
        BeaconPrintf(CALLBACK_ERROR, "Failed to get ToastNotificationFactory: 0x%08lx\n", (unsigned long)hr);
        goto fail_xmldoc;
    }

    IToastNotification *toast = NULL;
    hr = toastFactory->lpVtbl->CreateToastNotification(toastFactory, xmlDoc, &toast);
    toastFactory->lpVtbl->Release(toastFactory);
    xmlDoc->lpVtbl->Release(xmlDoc);
    xmlDoc = NULL;
    if (FAILED(hr)) {
        BeaconPrintf(CALLBACK_ERROR, "CreateToastNotification failed: 0x%08lx\n", (unsigned long)hr);
        goto fail_manager;
    }

    IToastNotifier *notifier = NULL;
    HSTRING aumidStr = make_hstring(aumid);
    if (!aumidStr) goto fail_toast;
    hr = toastManager->lpVtbl->CreateToastNotifierWithId(toastManager, aumidStr, &notifier);
    COMBASE$WindowsDeleteString(aumidStr);
    toastManager->lpVtbl->Release(toastManager);
    toastManager = NULL;
    if (FAILED(hr)) {
        BeaconPrintf(CALLBACK_ERROR, "CreateToastNotifierWithId failed: 0x%08lx\n", (unsigned long)hr);
        goto fail_toast;
    }

    hr = notifier->lpVtbl->Show(notifier, toast);
    notifier->lpVtbl->Release(notifier);
    toast->lpVtbl->Release(toast);
    if (FAILED(hr)) {
        BeaconPrintf(CALLBACK_ERROR, "Show failed: 0x%08lx\n", (unsigned long)hr);
        //if (roInitCalled) COMBASE$RoUninitialize();
        return 1;
    }

    BeaconPrintf(CALLBACK_OUTPUT, "Toast sent successfully.\n");
    //if (roInitCalled) COMBASE$RoUninitialize();
    return 0;

fail_xmlio:
    xmlIO->lpVtbl->Release(xmlIO);
fail_xmldoc:
    if (xmlDoc) xmlDoc->lpVtbl->Release(xmlDoc);
fail_manager:
    if (toastManager) toastManager->lpVtbl->Release(toastManager);
fail_toast:
    if (toast) toast->lpVtbl->Release(toast);
fail_early:
    //if (roInitCalled) COMBASE$RoUninitialize();
    return 1;
}

/* Wrapper semplice — costruisce XML da title+text e delega */
int sendToast(const wchar_t *aumid, const wchar_t *title, const wchar_t *text)
{
    const size_t xmlBufLen = 2048;
    wchar_t *xmlBuf = (wchar_t *)intAlloc(xmlBufLen * sizeof(wchar_t));
    if (!xmlBuf) {
        BeaconPrintf(CALLBACK_ERROR, "Failed to allocate xmlBuf\n");
        return 1;
    }

    MSVCRT$_snwprintf(xmlBuf, xmlBufLen,
        L"<toast>"
        L"<visual>"
        L"<binding template='ToastGeneric'>"
        L"<text>%ls</text>"
        L"<text>%ls</text>"
        L"</binding>"
        L"</visual>"
        L"</toast>",
        title, text);
    xmlBuf[xmlBufLen - 1] = L'\0';

    int ret = sendToastXml(aumid, xmlBuf);
    intFree(xmlBuf);
    return ret;
}

/* Wrapper custom — decode base64 -> wchar_t e delega */
int sendToastCustom(const wchar_t *aumid, const char *b64xml, int b64len)
{
    DWORD xmlLen = 0;

    /* Sizing call — length 0 = null-terminated input */
    if (!CRYPT32$CryptStringToBinaryA(b64xml, 0, CRYPT_STRING_BASE64,
                                      NULL, &xmlLen, NULL, NULL)) {
        BeaconPrintf(CALLBACK_ERROR, "CryptStringToBinaryA sizing failed: 0x%08lx\n",
                     (unsigned long)KERNEL32$GetLastError());
        return 1;
    }

    char *xmlUtf8 = (char *)intAlloc(xmlLen + 1);
    if (!xmlUtf8) {
        BeaconPrintf(CALLBACK_ERROR, "Failed to allocate xmlUtf8\n");
        return 1;
    }

    /* Decode */
    if (!CRYPT32$CryptStringToBinaryA(b64xml, 0, CRYPT_STRING_BASE64,
                                      (BYTE *)xmlUtf8, &xmlLen, NULL, NULL)) {
        BeaconPrintf(CALLBACK_ERROR, "CryptStringToBinaryA decode failed: 0x%08lx\n",
                     (unsigned long)KERNEL32$GetLastError());
        intFree(xmlUtf8);
        return 1;
    }
    xmlUtf8[xmlLen] = '\0';

    /* ASCII/UTF-8 -> wchar_t (manual, byte-by-byte) */
    wchar_t *xmlWide = (wchar_t *)intAlloc((xmlLen + 1) * sizeof(wchar_t));
    if (!xmlWide) {
        BeaconPrintf(CALLBACK_ERROR, "Failed to allocate xmlWide\n");
        intFree(xmlUtf8);
        return 1;
    }

    DWORD i;
    for (i = 0; i <= xmlLen; i++) {
        xmlWide[i] = (wchar_t)(unsigned char)xmlUtf8[i];
    }
    intFree(xmlUtf8);

    int ret = sendToastXml(aumid, xmlWide);
    intFree(xmlWide);
    return ret;
}

void go(char *args, int len)
{
    datap parser;
    char *cmd;

    BeaconDataParse(&parser, args, len);
    cmd = BeaconDataExtract(&parser, NULL);

    if (!cmd || !*cmd) {
        BeaconPrintf(CALLBACK_ERROR, "Usage: <getaumid|sendtoast|custom> [args...]\n");
        return;
    }

    if (!MSVCRT$strcmp(cmd, "getaumid")) {
        getAUMID();
        return;
    }

    if (!MSVCRT$strcmp(cmd, "sendtoast")) {
        int aumidLen = 0, titleLen = 0, textLen = 0;
        char *aumid_a = BeaconDataExtract(&parser, &aumidLen);
        char *title_a = BeaconDataExtract(&parser, &titleLen);
        char *text_a  = BeaconDataExtract(&parser, &textLen);

        if (!aumid_a || !title_a || !text_a) {
            BeaconPrintf(CALLBACK_ERROR, "sendtoast requires aumid, title, text\n");
            return;
        }

        wchar_t aumid[256], title[256], text_buf[1024];
        toWideChar(aumid_a, aumid, 256);
        toWideChar(title_a, title, 256);
        toWideChar(text_a,  text_buf, 1024);

        sendToast(aumid, title, text_buf);
        return;
    }

    if (!MSVCRT$strcmp(cmd, "custom")) {
        int aumidLen = 0, b64Len = 0;
        char *aumid_a = BeaconDataExtract(&parser, &aumidLen);
        char *b64xml  = BeaconDataExtract(&parser, &b64Len);

        if (!aumid_a || !b64xml) {
            BeaconPrintf(CALLBACK_ERROR, "custom toast requires aumid, b64xml\n");
            return;
        }

        wchar_t aumid[256];
        toWideChar(aumid_a, aumid, 256);

        sendToastCustom(aumid, b64xml, b64Len > 0 ? b64Len - 1 : 0);
        return;
    }

    BeaconPrintf(CALLBACK_ERROR, "Unknown command: %s\n", cmd);
}