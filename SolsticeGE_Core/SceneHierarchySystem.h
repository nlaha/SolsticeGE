#pragma once
#include "System.h"
#include "GameplayComponents.h"
#include "RenderComponents.h"
#include <glm/matrix.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/quaternion.hpp>

namespace SolsticeGE {
    class SceneHierarchySystem :
        public System
    {
    public:
        void update(entt::registry& registry);
    };
}

