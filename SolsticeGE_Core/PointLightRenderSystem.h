#pragma once
#include "System.h"

#include "RenderComponents.h"
#include "AABB.hpp"

namespace SolsticeGE {
    class PointLightRenderSystem :
        public System
    {
    public:
        void update(entt::registry& registry);
    };
}
