#include "CameraRenderSystem.h"

using namespace SolsticeGE;

void CameraRenderSystem::update(entt::registry& registry)
{
	auto ecs_view = registry.view<
		const c_transform,
		const c_camera
	>();

	for (const auto& [entity, transform, camera] : ecs_view.each())
	{
		glm::mat4 modelMatrix;

		// compute matrix;
		modelMatrix = glm::toMat4(transform.rot);
		modelMatrix = glm::translate(modelMatrix, transform.pos);
		modelMatrix = glm::scale(modelMatrix, transform.scale);

		// convert model matrix to view matrix
		glm::mat4 viewMatrix = glm::inverse(modelMatrix);
		glm::mat4 projMatrix = glm::perspectiveFov(
			camera.fov,
			camera.size.x,
			camera.size.y,
			camera.clipNear,
			camera.clipFar
		);

		bgfx::setViewTransform(0, &viewMatrix[0][0], &projMatrix[0][0]);

		// Set view 0 default viewport.
		bgfx::setViewRect(0, 0, 0,
			static_cast<uint16_t>(camera.size.x),
			static_cast<uint16_t>(camera.size.y));
	}
}
