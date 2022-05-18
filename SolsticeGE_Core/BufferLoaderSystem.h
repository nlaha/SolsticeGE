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

        void moveCubemapToGPU(const ASSET_ID& texture);

        void moveTextureToGPU(const ASSET_ID& texture);

    private:
        int m_texCount;
        int m_cubemapCount;
    };
}