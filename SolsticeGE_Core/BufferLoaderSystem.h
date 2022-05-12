#pragma once
#include <entt/entt.hpp>
#include <sstream>

#include "System.h"
#include "RenderComponents.h"

namespace SolsticeGE {
    class BufferLoaderSystem :
        public System
    {
        void update(entt::registry& registry);

        void moveTextureToGPU(const std::string& texture);
    };
}