#include "PointLightRenderSystem.h"
#include "EngineWrapper.h"

using namespace SolsticeGE;
using namespace CPM_GLM_AABB_NS;

void PointLightRenderSystem::update(entt::registry& registry)
{
	auto ecs_view = registry.view<
		const c_light_point,
		const c_transform
	>();

	const auto& activeCamera = registry.get<c_camera>(EngineWrapper::activeCamera);

	glm::mat4 viewMatrix = activeCamera.viewMatrix;
	glm::mat4 projMatrix = activeCamera.projMatrix;
	glm::mat4 viewProjMatrix = viewMatrix * projMatrix;

	for (const auto& [entity, light, transform] : ecs_view.each())
	{
		float winWidth = EngineWrapper::videoSettings.windowWidth;
		float winHeight = EngineWrapper::videoSettings.windowHeight;

		AABB aabb;
		aabb.translate(transform.pos);
		aabb.scale(glm::vec3(light.outer), transform.pos);

		const glm::vec3 box[8] =
		{
			{ aabb.getMin().x, aabb.getMin().y, aabb.getMin().z},
			{ aabb.getMin().x, aabb.getMin().y, aabb.getMax().z },
			{ aabb.getMin().x, aabb.getMax().y, aabb.getMin().z },
			{ aabb.getMin().x, aabb.getMax().y, aabb.getMax().z },
			{ aabb.getMax().x, aabb.getMin().y, aabb.getMin().z},
			{ aabb.getMax().x, aabb.getMin().y, aabb.getMax().z },
			{ aabb.getMax().x, aabb.getMax().y, aabb.getMin().z },
			{ aabb.getMax().x, aabb.getMax().y, aabb.getMax().z },
		};

		glm::vec3 xyz = glm::vec3(glm::vec4(box[0], 0.0f) * viewProjMatrix);
		glm::vec3 min = xyz;
		glm::vec3 max = xyz;

		for (uint32_t ii = 1; ii < 8; ++ii)
		{
			xyz = glm::vec3(glm::vec4(box[ii], 0.0f) * viewProjMatrix);
			min = glm::min(min, xyz);
			max = glm::max(max, xyz);
		}

		if (max.z >= 0.0f)
		{
			const float x0 = std::clamp<float>((min.x * 0.5f + 0.5f) * winWidth, 0.0f, (float)winWidth);
			const float y0 = std::clamp<float>((min.y * 0.5f + 0.5f) * winHeight, 0.0f, (float)winHeight);
			const float x1 = std::clamp<float>((max.x * 0.5f + 0.5f) * winWidth, 0.0f, (float)winWidth);
			const float y1 = std::clamp<float>((max.y * 0.5f + 0.5f) * winHeight, 0.0f, (float)winHeight);
			
			bgfx::setUniform(EngineWrapper::shaderUniforms.at("lightPositions"),
				&transform.pos[0]);
			bgfx::setUniform(EngineWrapper::shaderUniforms.at("lightColors"),
				&light.color[0]);

			const uint16_t scissorHeight = uint16_t(y1 - y0);
			bgfx::setScissor(uint16_t(x0), uint16_t(winHeight - scissorHeight - y0), uint16_t(x1 - x0), uint16_t(scissorHeight));

			// render light pass
			bgfx::setTexture(0,
				EngineWrapper::shaderSamplers.at("albedo"),
				bgfx::getTexture(EngineWrapper::gbuffer, 0));
			bgfx::setTexture(1,
				EngineWrapper::shaderSamplers.at("depth"),
				bgfx::getTexture(EngineWrapper::gbuffer, 1));
			bgfx::setTexture(2,
				EngineWrapper::shaderSamplers.at("normal"),
				bgfx::getTexture(EngineWrapper::gbuffer, 2));
			bgfx::setTexture(3,
				EngineWrapper::shaderSamplers.at("position"),
				bgfx::getTexture(EngineWrapper::gbuffer, 3));
			bgfx::setState(0
				| BGFX_STATE_WRITE_RGB
				| BGFX_STATE_WRITE_A
			);

			EngineWrapper::screenSpaceQuad(
				winWidth,
				winHeight,
				EngineWrapper::texelHalf,
				EngineWrapper::renderCaps->originBottomLeft);
			bgfx::submit(kRenderPassLight, EngineWrapper::lightProgram);
		}
	}
}
