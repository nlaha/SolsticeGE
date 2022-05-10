#include "MeshRenderSystem.h"

using namespace SolsticeGE;

void MeshRenderSystem::update(entt::registry& registry)
{
	auto ecs_view = registry.view<
		const c_transform,
		const c_mesh, 
		const c_shader
	>();

	uint64_t state = 0
		| BGFX_STATE_WRITE_R
		| BGFX_STATE_WRITE_G
		| BGFX_STATE_WRITE_B
		| BGFX_STATE_WRITE_A
		| BGFX_STATE_WRITE_Z
		| BGFX_STATE_DEPTH_TEST_LESS
		| BGFX_STATE_CULL_CW
		| BGFX_STATE_MSAA
		| UINT64_C(0)
		;

	for (const auto& [entity, transform, mesh, shader] : ecs_view.each())
	{
		if (mesh.isValid) {

			glm::mat4 modelMatrix;

			// compute matrix;
			modelMatrix = glm::toMat4(transform.rot);
			modelMatrix = glm::translate(modelMatrix, transform.pos);
			modelMatrix = glm::scale(modelMatrix, transform.scale);

			bgfx::setTransform(&modelMatrix[0][0]);
		
			bgfx::setVertexBuffer(0, mesh.vbuf);
			bgfx::setIndexBuffer(mesh.ibuf);

			bgfx::setState(state);

			bgfx::submit(0, shader.program);
		}
	}
}
