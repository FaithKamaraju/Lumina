//
// Created by Faith Kamaraju on 2026-03-05.
//

#pragma once
#include "ECS/Entity.h"

namespace LE {

    class NativeScript {

    public:

    #define LE_SCRIPT_CLASS(TYPE) \
        public: \
        static const char* StaticScriptName() { return #TYPE; }

        virtual ~NativeScript() = default;

        void SetEntity(Entity entity) { mEntity = entity; }
        Entity GetEntity() const { return mEntity; }

        virtual void OnCreate() {}
        virtual void OnDestroy() {}
        virtual void OnUpdate(float ts) {}

    private:
        Entity mEntity{};

    };

}
