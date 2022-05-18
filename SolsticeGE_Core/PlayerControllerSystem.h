#pragma once
#include "System.h"
#include "RenderComponents.h"
#include "GameplayComponents.h"
#include "EngineWrapper.h"

#include <glm/vec3.hpp>
#include <glm/gtc/quaternion.hpp>

namespace SolsticeGE {
    class PlayerControllerSystem :
        public System
    {
    public:
        PlayerControllerSystem();

        void update(entt::registry& registry);

    private:
        double m_camYaw, m_camPitch;
    };
}

