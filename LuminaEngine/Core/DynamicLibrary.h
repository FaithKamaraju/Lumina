//
// Created by Faith Kamaraju on 2026-03-05.
//


#pragma once

#include <filesystem>
#include <string>

namespace LE {

    class DynamicLibrary {
    public:
        DynamicLibrary() = default;
        ~DynamicLibrary() { Unload(); }

        DynamicLibrary(const DynamicLibrary&) = delete;
        DynamicLibrary& operator=(const DynamicLibrary&) = delete;

        DynamicLibrary(DynamicLibrary&& other) noexcept { *this = std::move(other); }
        DynamicLibrary& operator=(DynamicLibrary&& other) noexcept
        {
            if (this == &other) return *this;
            Unload();
            m_Handle = other.m_Handle;
            m_Path = std::move(other.m_Path);
            other.m_Handle = nullptr;
            return *this;
        }

        bool Load(const std::filesystem::path& path, std::string* outError = nullptr);
        void Unload();

        [[nodiscard]] bool IsLoaded() const { return m_Handle != nullptr; }
        [[nodiscard]] const std::filesystem::path& GetPath() const { return m_Path; }

        template<typename T>
        T GetSymbol(const char* name, std::string* outError = nullptr) const
        {
            void* sym = GetSymbolRaw(name, outError);
            return reinterpret_cast<T>(sym);
        }

    private:
        void* GetSymbolRaw(const char* name, std::string* outError) const;

    private:
        void* m_Handle = nullptr;                 // HMODULE on Win, void* on macOS
        std::filesystem::path m_Path{};
    };

} // namespace LE