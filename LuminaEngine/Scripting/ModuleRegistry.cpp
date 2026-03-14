//
// Created by Faith Kamaraju on 2026-03-05.
//

#include "ModuleRegistry.h"


LE::ModuleRegistry::ModuleRegistry() {
}

LE::ModuleRegistry::~ModuleRegistry() {
}

void LE::ModuleRegistry::RegisterScriptType(const ScriptTypeRegistration &info) {

    uint64_t hashedName = std::hash<const char*>{}(info.name);
    mScriptsRegistry[info.name] = {hashedName, info.Create, info.Destroy};

}

LE::ScriptRecord LE::ModuleRegistry::GetScriptRecord(const std::string& scriptName) {

    auto it = mScriptsRegistry.find(scriptName);
    if (it == mScriptsRegistry.end()) return {};
    return it->second;

}

