#pragma once
#include "System.h"

#include <spdlog/spdlog.h>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "RenderComponents.h"
#include "RenderCommon.h"

namespace SolsticeGE {
    class ModelLoaderSystem :
        public System
    {
    public:
        void update(entt::registry& registry);

    };
}
