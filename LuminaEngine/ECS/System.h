//
// Created by Faith Kamaraju on 2025-12-21.
//

#pragma once

#include <set>
#include "ECS/Entity.h"

namespace LE {

    class System
    {
    public:
        virtual ~System() = default;
        virtual void SetSystemSignature() = 0;

        std::set<Entity> mEntities;
    };

}