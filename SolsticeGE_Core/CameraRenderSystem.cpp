#include "CameraRenderSystem.h"

#include "EngineWrapper.h"

using namespace SolsticeGE;

void CameraRenderSystem::update(entt::registry& registry)
{
	auto ecs_view = registry.view<
		c_transform,
		c_camera
	>();

	for (const auto& entity : ecs_view)
	{
		auto& transform = ecs_view.get<c_transform>(entity);
		auto& camera = ecs_view.get<c_camera>(entity);

		glm::mat4 modelMatrix;

		camera.size.x = EngineWrapper::videoSettings.windowWidth;
		camera.size.y = EngineWrapper::videoSettings.windowHeight;

		// compute matrix;
		modelMatrix = glm::toMat4(transform.rot);
		modelMatrix = glm::translate(modelMatrix, transform.pos);
		modelMatrix = glm::scale(modelMatrix, transform.scale);

		// convert model matrix to view matrix
		glm::mat4 viewMatrix = glm::inverse(modelMatrix);

		for (RenderPass pass : camera.passes) {
			glm::mat4 projMatrix;

			bgfx::setViewRect(pass.viewId, 0, 0,
				static_cast<uint16_t>(camera.size.x),
				static_cast<uint16_t>(camera.size.y));

			float* viewMat = &viewMatrix[0][0];
			float* projMat = nullptr;
			if (pass.fullscreenOrtho) {
				viewMat = nullptr;
				projMatrix = glm::ortho(0.0f,
					1.0f,
					1.0f, 0.0f, -100.0f, 100.0f);
			}
			else {
				projMatrix = glm::perspectiveFov(
					camera.fov,
					camera.size.x,
					camera.size.y,
					camera.clipNear,
					camera.clipFar
				);
			}

			projMat = &projMatrix[0][0];

			if (pass.viewId == kRenderPassGeometry)
			{
				bgfx::setUniform(EngineWrapper::shaderUniforms.at("viewPos"), &transform.pos[0]);
			}

			camera.viewMatrix = viewMatrix;
			camera.projMatrix = projMatrix;

			bgfx::setViewTransform(pass.viewId, viewMat, projMat);
		}
	}
}
