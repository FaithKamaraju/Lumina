//
// Created by Faith Kamaraju on 2026-01-15.
//

#pragma once
#include "mini/ini.h"

namespace LE {

    struct WindowConfig {
        int width = 1280;
        int height = 720;
        std::string title = "Lumina Game";
        bool vsync = true;
        bool fullscreen = false;

        WindowConfig() = default;
        explicit WindowConfig(mINI::INIStructure& ini) {
            width = std::stoi(ini["WindowSettings"]["width"]);
            height = std::stoi(ini["WindowSettings"]["height"]);
            title = ini["WindowSettings"]["title"];
            vsync = std::stoi(ini["WindowSettings"]["enableVsync"]) > 0;
            fullscreen = std::stoi(ini["WindowSettings"]["fullscreen"]) > 0;
        }
        void toMap(mINI::INIStructure& ini) {
            ini["WindowSettings"]["width"] = std::to_string(width);
            ini["WindowSettings"]["height"] = std::to_string(height);
            ini["WindowSettings"]["title"] = title;
            ini["WindowSettings"]["enableVsync"] = std::to_string(vsync ? 1:0);
            ini["WindowSettings"]["fullscreen"] = std::to_string(fullscreen ? 1:0);
        }

    };

    struct Config {
        bool changed = false;
        WindowConfig window_config{};

    };

    Config LoadConfig(const char* path);
    void SaveConfig(const char* path, Config& config);
    void convertMapToStruct(Config& config, mINI::INIStructure& ini);
    void convertStructToMap(mINI::INIStructure& ini, Config& config);


}
