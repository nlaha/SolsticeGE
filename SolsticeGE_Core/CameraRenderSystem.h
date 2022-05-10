#pragma once
#include "System.h"
#include "RenderComponents.h"
#include <glm/matrix.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/quaternion.hpp>

namespace SolsticeGE {
    class CameraRenderSystem :
        public System
    {
    public:
        void update(entt::registry& registry);
    };
}

