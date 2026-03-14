//
// Created by Faith Kamaraju on 2026-03-01.
//

#pragma once

#include <filesystem>

namespace LE {

    namespace FileSystem {

        namespace fs = std::filesystem;

        inline fs::path MakeUniqueNameBySuffix(fs::path filePath) {

            filePath = filePath.lexically_normal();

            if (!fs::exists(filePath))
                return filePath;

            const fs::path dir = filePath.parent_path();
            const std::string stem = filePath.stem().string();
            const std::string ext  = filePath.extension().string();

            for (int i = 1; i < 1000000; ++i) {
                fs::path candidate = dir / fs::path(stem + " (" + std::to_string(i) + ")" + ext);
                if (!fs::exists(candidate))
                    return candidate;
            }

            throw std::runtime_error("Could not find a unique filename for: " + filePath.string());
        }
    }



}
