#pragma once
#include "System.h"

#include "RenderComponents.h"
#include "AABB.hpp"

namespace SolsticeGE {
    class LightRenderSystem :
        public System
    {
    public:
        void update(entt::registry& registry);
    };
}
