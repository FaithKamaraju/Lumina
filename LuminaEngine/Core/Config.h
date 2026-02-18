//
// Created by Faith Kamaraju on 2026-01-15.
//

#pragma once
#include "mini/ini.h"
#include "LE_Types.h"

namespace LE {

    struct WindowConfig {
        int width = 1280;
        int height = 720;
        bool vsync = true;
        bool fullscreen = false;
        std::string title = "Lumina Game";

        WindowConfig() = default;
        explicit WindowConfig(mINI::INIStructure& ini) {
            width = std::stoi(ini["WindowSettings"]["width"]);
            height = std::stoi(ini["WindowSettings"]["height"]);
            title = ini["WindowSettings"]["title"];
            vsync = std::stoi(ini["WindowSettings"]["enableVsync"]) > 0;
            fullscreen = std::stoi(ini["WindowSettings"]["fullscreen"]) > 0;
        }
        void toMap(mINI::INIStructure& ini) const {
            ini["WindowSettings"]["width"] = std::to_string(width);
            ini["WindowSettings"]["height"] = std::to_string(height);
            ini["WindowSettings"]["title"] = title;
            ini["WindowSettings"]["enableVsync"] = std::to_string(vsync ? 1:0);
            ini["WindowSettings"]["fullscreen"] = std::to_string(fullscreen ? 1:0);
        }

    };

    struct GraphicsConfig {

        GraphicsAPI graphicsAPI = GraphicsAPI::VULKAN;

        GraphicsConfig() = default;
        explicit GraphicsConfig(mINI::INIStructure& ini) {
            graphicsAPI = static_cast<GraphicsAPI>(std::stoi(ini["GraphicsSettings"]["graphicsAPI"]));

        }
        void toMap(mINI::INIStructure& ini) const {
            ini["GraphicsSettings"]["graphicsAPI"] = std::to_string(static_cast<int>(graphicsAPI));

        }

    };

    struct Config {
        WindowConfig window_config{};
        GraphicsConfig graphics_config{};
        bool changed = false;

    };

    Config LoadConfig(const char* path);
    void SaveConfig(const char* path, Config& config);
    void convertMapToStruct(Config& config, mINI::INIStructure& ini);
    void convertStructToMap(mINI::INIStructure& ini, Config& config);


}
