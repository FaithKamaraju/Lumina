//
// Created by Faith Kamaraju on 2026-01-15.
//

#include "Config.h"
#include "Logger.h"

LE::Config LE::LoadConfig(const char* path) {

    Config cfg{};
    mINI::INIFile file(path);
    mINI::INIStructure ini;
    bool result = file.read(ini);

    if (!result) {
        LE_WARN("Couldn't read config ini file from path {0}, Loading default values",path);
        convertStructToMap(ini, cfg);
        file.write(ini, true);
    }
    else {
        convertMapToStruct(cfg, ini);
    }

    return cfg;
}

void LE::SaveConfig(const char *path, Config& config) {
    config.changed = false;
    mINI::INIFile file(path);
    mINI::INIStructure ini;
    convertStructToMap(ini, config);
    file.write(ini, true);
}

void LE::convertMapToStruct(Config& config, mINI::INIStructure& ini) {

    config.window_config = WindowConfig{ini};
}

void LE::convertStructToMap(mINI::INIStructure &ini, Config &config) {

    config.window_config.toMap(ini);
}
