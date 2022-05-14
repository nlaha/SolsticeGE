#pragma once
#include <entt/entt.hpp>
#include <sstream>

#include "System.h"
#include "RenderComponents.h"

namespace SolsticeGE {
    class BufferLoaderSystem :
        public System
    {
    public:
        BufferLoaderSystem();

        void update(entt::registry& registry);

        void moveTextureToGPU(const ASSET_ID& texture);

    private:
        int m_texCount;
    };
}