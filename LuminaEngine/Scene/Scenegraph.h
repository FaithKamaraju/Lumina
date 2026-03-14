//
// Created by Faith Kamaraju on 2026-02-01.
//

#pragma once
#include "Scene.h"
#include "Rendering/Renderer.h"

namespace LE {

    class Scenegraph {
    public:

        Scenegraph();

        void AddSceneAtIndex(std::vector<SceneNode>& nodes, int32_t parentIndex, int32_t previousSiblingIndex = -1 );
        void AddNodeAtIndex(SceneNode& node, int32_t parentIndex, int32_t previousSiblingIndex = -1);

    private:

        std::vector<Hierarchy> mNodeHierarchies;

        friend class Renderer;

    };
}
