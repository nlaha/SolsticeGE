#include "PlayerControllerSystem.h"

using namespace SolsticeGE;

PlayerControllerSystem::PlayerControllerSystem()
{
	m_camYaw = 0.0;
	m_camPitch = 0.0;
}

void PlayerControllerSystem::update(entt::registry& registry)
{
	auto player_view = registry.view<
		c_transform,
		c_player,
		c_camera
	>();

	// ====== MOUSE LOOK ======
	glm::vec3 cameraFront = EngineWrapper::userInput.cameraFront;
	glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
	glm::vec3 cameraRight = glm::normalize(glm::cross(cameraUp, cameraFront));
	cameraUp = glm::cross(cameraFront, cameraRight);

	for (const auto& entity : player_view)
	{
		auto& transform = player_view.get<c_transform>(entity);
		auto& camera = player_view.get<c_camera>(entity);

		glm::quat lookRot = glm::quatLookAt(cameraFront, cameraUp);
		camera.viewMatrix = glm::lookAt(transform.pos, transform.pos + cameraFront, cameraUp);

		if (InputManager::isBtnDown(GLFW_KEY_W))
		{
			transform.pos += cameraFront * EngineWrapper::dt;
		}

		if (InputManager::isBtnDown(GLFW_KEY_S))
		{
			transform.pos -= cameraFront * EngineWrapper::dt;
		}

		if (InputManager::isBtnDown(GLFW_KEY_D))
		{
			transform.pos += glm::normalize(glm::cross(cameraFront, cameraUp)) * EngineWrapper::dt;
		}

		if (InputManager::isBtnDown(GLFW_KEY_A))
		{
			transform.pos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * EngineWrapper::dt;
		}
	}
}
