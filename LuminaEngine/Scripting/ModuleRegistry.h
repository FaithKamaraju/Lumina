//
// Created by Faith Kamaraju on 2026-03-05.
//

#pragma once

#include "NativeScript.h"
#include <unordered_map>

namespace LE {

    using CreateScriptFn = NativeScript* (*)();
    using DestroyScriptFn = void (*)(NativeScript*);

    struct ScriptTypeRegistration
    {
        const char* name{};
        CreateScriptFn Create = nullptr;
        DestroyScriptFn Destroy = nullptr;
    };

    struct ScriptRecord
    {
        uint64_t hashedName{};
        CreateScriptFn Create = nullptr;
        DestroyScriptFn Destroy = nullptr;
    };

    class ModuleRegistry {

    public:

        ModuleRegistry();
        ~ModuleRegistry();

        template<class T>
        void RegisterScript() {
            RegisterScriptType({
                T::StaticScriptName(),
                []() -> NativeScript* { return new T(); },
                [](NativeScript* script) { delete static_cast<T*>(script); }
            });
        }

        void RegisterScriptType(const ScriptTypeRegistration& info);
        ScriptRecord GetScriptRecord(const std::string& scriptName);

        // NativeScript*

    private:

        std::unordered_map<std::string, ScriptRecord> mScriptsRegistry{};

    };

}
