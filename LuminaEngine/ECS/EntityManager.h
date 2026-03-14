//
// Created by Faith Kamaraju on 2025-12-18.
//

#pragma once

#include "ECS/Entity.h"
namespace LE {
    class EntityManager
    {
    public:
        EntityManager();
        Entity CreateEntity();
        void DestroyEntity(Entity entity);
        void SetSignature(Entity entity, Signature signature);
        Signature GetSignature(Entity entity);

    private:
        // Queue of unused entity IDs
        std::queue<Entity> mAvailableEntities{};

        // Array of signatures where the index corresponds to the entity ID
        std::array<Signature, MAX_ENTITIES> mSignatures{};

        // Total living entities - used to keep limits on how many exist
        uint32_t mLivingEntityCount{};
    };
}
