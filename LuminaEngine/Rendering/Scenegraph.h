//
// Created by Faith Kamaraju on 2026-02-01.
//

#pragma once
#include "Resource/Scene.h"

namespace LE {

    class Scenegraph {

    public:
        Scenegraph();
        ~Scenegraph();



    private:

        std::vector<SceneNode> mNodes;

    };
}
