#pragma once
#include "System.h"
#include "RenderComponents.h"
#include "AssetLibrary.h"

namespace SolsticeGE {
    class SceneSpawnerSystem :
        public System
    {
    public:
        void update(entt::registry& registry);
    };
}

