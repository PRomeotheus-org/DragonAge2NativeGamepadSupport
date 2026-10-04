// Dragon Age II -- console controls and console UI for the PC build.
// Release build: no diagnostic output. See the development source for why any
// of this is shaped the way it is.

#include <windows.h>

void StartConsoleMod();

namespace {

HMODULE g_real = nullptr;

using PFN_DirectInput8Create = HRESULT (WINAPI*)(HINSTANCE, DWORD, REFIID, LPVOID*, LPUNKNOWN);
using PFN_DllCanUnloadNow    = HRESULT (WINAPI*)();
using PFN_DllGetClassObject  = HRESULT (WINAPI*)(REFCLSID, REFIID, LPVOID*);
using PFN_DllRegisterServer  = HRESULT (WINAPI*)();
using PFN_DllUnregisterServer= HRESULT (WINAPI*)();

PFN_DirectInput8Create  p_DirectInput8Create   = nullptr;
PFN_DllCanUnloadNow     p_DllCanUnloadNow      = nullptr;
PFN_DllGetClassObject   p_DllGetClassObject    = nullptr;
PFN_DllRegisterServer   p_DllRegisterServer    = nullptr;
PFN_DllUnregisterServer p_DllUnregisterServer  = nullptr;

void Fail(const wchar_t* what) {

    MessageBoxW(nullptr, what, L"Dragon Age II console mod", MB_ICONERROR | MB_OK);
}

bool LoadReal() {
    wchar_t path[MAX_PATH];
    const UINT n = GetSystemDirectoryW(path, MAX_PATH);
    if (!n || n >= MAX_PATH - 16) {
        Fail(L"GetSystemDirectoryW failed - cannot locate the real dinput8.dll.");
        return false;
    }
    lstrcatW(path, L"\\dinput8.dll");

    g_real = LoadLibraryW(path);
    if (!g_real) {
        Fail(L"Could not load the system dinput8.dll.\n\n"
             L"On Steam Deck / Proton this usually means the launch option is "
             L"missing:\n\n    WINEDLLOVERRIDES=\"dinput8=n,b\" %command%");
        return false;
    }

    p_DirectInput8Create  = (PFN_DirectInput8Create) GetProcAddress(g_real, "DirectInput8Create");
    p_DllCanUnloadNow     = (PFN_DllCanUnloadNow)    GetProcAddress(g_real, "DllCanUnloadNow");
    p_DllGetClassObject   = (PFN_DllGetClassObject)  GetProcAddress(g_real, "DllGetClassObject");
    p_DllRegisterServer   = (PFN_DllRegisterServer)  GetProcAddress(g_real, "DllRegisterServer");
    p_DllUnregisterServer = (PFN_DllUnregisterServer)GetProcAddress(g_real, "DllUnregisterServer");

    if (!p_DirectInput8Create) {
        Fail(L"The system dinput8.dll has no DirectInput8Create entry point.");
        return false;
    }
    return true;
}

}

extern "C" {

HRESULT WINAPI DirectInput8Create(HINSTANCE hinst, DWORD ver, REFIID riid,
                                  LPVOID* out, LPUNKNOWN outer) {
    if (!p_DirectInput8Create) return E_FAIL;
    return p_DirectInput8Create(hinst, ver, riid, out, outer);
}

HRESULT WINAPI DllCanUnloadNow() {

    return p_DllCanUnloadNow ? p_DllCanUnloadNow() : S_FALSE;
}

HRESULT WINAPI DllGetClassObject(REFCLSID clsid, REFIID riid, LPVOID* out) {
    if (!p_DllGetClassObject) return CLASS_E_CLASSNOTAVAILABLE;
    return p_DllGetClassObject(clsid, riid, out);
}

HRESULT WINAPI DllRegisterServer() {
    return p_DllRegisterServer ? p_DllRegisterServer() : E_FAIL;
}

HRESULT WINAPI DllUnregisterServer() {
    return p_DllUnregisterServer ? p_DllUnregisterServer() : E_FAIL;
}

}

BOOL APIENTRY DllMain(HMODULE h, DWORD reason, LPVOID) {
    if (reason == DLL_PROCESS_ATTACH) {
        DisableThreadLibraryCalls(h);

        if (!LoadReal()) return FALSE;

        StartConsoleMod();
    }
    return TRUE;
}
