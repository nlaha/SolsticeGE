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

        void moveTextureToGPU(const std::weak_ptr<AssetLibrary::Texture>& texture, const std::uint16_t& id);
    };
}