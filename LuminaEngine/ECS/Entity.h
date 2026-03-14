//
// Created by Faith Kamaraju on 2025-12-18.
//

#pragma once
#include <bitset>

// A simple type alias
using Entity = std::uint32_t;

// Used to define the size of arrays later on
constexpr Entity MAX_ENTITIES = 5000;

// A simple type alias
using ComponentType = std::uint8_t;

// Used to define the size of arrays later on
constexpr ComponentType MAX_COMPONENTS = 32;

// A simple type alias
using Signature = std::bitset<MAX_COMPONENTS>;