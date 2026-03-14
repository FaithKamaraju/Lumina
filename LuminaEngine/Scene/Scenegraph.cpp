//
// Created by Faith Kamaraju on 2026-02-01.
//

#include "Scenegraph.h"

#include "Core/EngineStatics.h"
#include "ECS/Coordinator.h"
#include "Core/Logger.h"

LE::Scenegraph::Scenegraph() {

    // Adding a default root component.
    mNodeHierarchies.reserve(MAX_ENTITIES);
    mNodeHierarchies.push_back({});
    mNodeHierarchies[0].entityID = Globals::GetCoordinator()->CreateEntity();
    Globals::GetCoordinator()->AddComponent(mNodeHierarchies[0].entityID, TransformComponent{});
    Globals::GetCoordinator()->AddComponent(mNodeHierarchies[0].entityID, MeshAssetHandle{});
}

void LE::Scenegraph::AddSceneAtIndex(std::vector<SceneNode> &nodes, int32_t parentIndex, int32_t previousSiblingIndex) {
    int firstNode = nodes[0].firstChild;

    auto traverse = [&](int firstNodeInLevel, int parent, int previousSibling, auto& self) -> void {
        for (int i = firstNodeInLevel; i != -1; i = nodes[i].nextSibling) {
            auto& node = nodes[i];
            AddNodeAtIndex(node, parent, previousSibling);
            previousSibling = mNodeHierarchies.size() - 1;
            if (node.firstChild != -1) {
                self(node.firstChild, mNodeHierarchies.size() - 1, -1,  self);
            }
        }
    };

    traverse(firstNode, parentIndex, previousSiblingIndex, traverse);

}

void LE::Scenegraph::AddNodeAtIndex(SceneNode &node, int32_t parentIndex, int32_t previousSiblingIndex) {
    LE_ASSERT(parentIndex < static_cast<int>(mNodeHierarchies.size()) && "Invalid insertion, parent index not valid!");
    LE_ASSERT(previousSiblingIndex < static_cast<int>(mNodeHierarchies.size()) && "Invalid insertion, previous sibling index not valid!");
    LE_ASSERT(previousSiblingIndex >= mNodeHierarchies[parentIndex].firstChild && "Invalid insertion, previous sibling index not valid!");

    int32_t newIndex = mNodeHierarchies.size();
    Hierarchy hrchy{};
    mNodeHierarchies.push_back(hrchy);



    auto& parentNode = mNodeHierarchies[parentIndex];
    auto& newNode = mNodeHierarchies[newIndex];
    newNode.parent = parentIndex;
    newNode.level = parentNode.level + 1;
    newNode.firstChild = -1;
    newNode.lastChild = -1;
    newNode.debug_name = node.debug_name;
    newNode.entityID = Globals::GetCoordinator()->CreateEntity();
    Globals::GetCoordinator()->AddComponent(newNode.entityID,
        TransformComponent {
        .localTransform =  node.localTransform,
        .globalTransform =  node.globalTransform}
        );
    Globals::GetCoordinator()->AddComponent(newNode.entityID, node.meshHandle);\
    if (previousSiblingIndex == -1) {
        previousSiblingIndex = parentNode.lastChild;
        parentNode.lastChild = newIndex;
    }
    if (parentNode.firstChild == -1) {
        newNode.nextSibling = -1;
        parentNode.firstChild = newIndex;
        parentNode.lastChild = newIndex;
    }
    else {
        newNode.nextSibling = mNodeHierarchies[previousSiblingIndex].nextSibling;
        mNodeHierarchies[previousSiblingIndex].nextSibling = newIndex;
    }


}
