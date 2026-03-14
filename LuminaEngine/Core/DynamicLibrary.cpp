//
// Created by Faith Kamaraju on 2026-03-05.
//

#include "DynamicLibrary.h"

#if defined(LE_PLATFORM_WIN)
    #define NOMINMAX
    #include <Windows.h>
#elif defined(LE_PLATFORM_MAC)
    #include <dlfcn.h>
#endif

namespace LE {

static void SetError(std::string* outError, const std::string& msg)
{
    if (outError) *outError = msg;
}

bool DynamicLibrary::Load(const std::filesystem::path& path, std::string* outError)
{
    Unload();
    m_Path = path;

#if defined(LE_PLATFORM_WIN)
    // LoadLibraryW wants a wide string path
    HMODULE h = ::LoadLibraryW(m_Path.wstring().c_str());
    if (!h) {
        DWORD err = ::GetLastError();
        SetError(outError, "LoadLibraryW failed (GetLastError=" + std::to_string(err) + ")");
        return false;
    }
    m_Handle = reinterpret_cast<void*>(h);
    return true;

#elif defined(LE_PLATFORM_MAC)
    // Clear old errors
    (void)::dlerror();

    void* h = ::dlopen(m_Path.string().c_str(), RTLD_NOW);
    if (!h) {
        const char* e = ::dlerror();
        SetError(outError, std::string("dlopen failed: ") + (e ? e : "unknown"));
        return false;
    }
    m_Handle = h;
    return true;
#else
    SetError(outError, "DynamicLibrary::Load not implemented for this platform");
    return false;
#endif
}

void DynamicLibrary::Unload()
{
    if (!m_Handle) return;

#if defined(LE_PLATFORM_WIN)
    ::FreeLibrary(reinterpret_cast<HMODULE>(m_Handle));
#elif defined(LE_PLATFORM_MAC)
    ::dlclose(m_Handle);
#endif

    m_Handle = nullptr;
    m_Path.clear();
}

void* DynamicLibrary::GetSymbolRaw(const char* name, std::string* outError) const
{
    if (!m_Handle) {
        SetError(outError, "Library not loaded");
        return nullptr;
    }

#if defined(LE_PLATFORM_WIN)
    FARPROC p = ::GetProcAddress(reinterpret_cast<HMODULE>(m_Handle), name);
    if (!p) {
        DWORD err = ::GetLastError();
        SetError(outError, std::string("GetProcAddress failed for symbol '") + name +
                          "' (GetLastError=" + std::to_string(err) + ")");
        return nullptr;
    }
    return reinterpret_cast<void*>(p);

#elif defined(LE_PLATFORM_MAC)
    (void)::dlerror(); // clear
    void* p = ::dlsym(m_Handle, name);
    const char* e = ::dlerror();
    if (e) {
        SetError(outError, std::string("dlsym failed for symbol '") + name + "': " + e);
        return nullptr;
    }
    return p;
#else
    SetError(outError, "DynamicLibrary::GetSymbol not implemented for this platform");
    return nullptr;
#endif
}

} // namespace LE