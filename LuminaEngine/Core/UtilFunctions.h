//
// Created by Faith Kamaraju on 2026-02-09.
//

#pragma once

namespace LE {

    inline void HashCombine(size_t& seed, size_t value) {

        seed ^= value + 0x9e3779b97f4a7c15ull + (seed << 6) + (seed >> 2);
    }
}