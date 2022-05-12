#include "MeshRenderSystem.h"
#include "EngineWrapper.h"

using namespace SolsticeGE;

MeshRenderSystem::MeshRenderSystem()
{
	angle = 0;
}

void MeshRenderSystem::update(entt::registry& registry)
{
	auto ecs_view = registry.view<
		const c_transform,
		const c_mesh, 
		const c_shader,
		const c_material
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

	for (const auto& [entity, transform, mesh, shader, material] : ecs_view.each())
	{
		std::weak_ptr<AssetLibrary::Mesh> meshAsset;
		if (!EngineWrapper::assetLib.getMesh(mesh.assetId, meshAsset))
		{
			continue;
		}

		if (meshAsset.lock()->bufferLoaded) {

			glm::mat4 modelMatrix;

			// compute matrix;
			modelMatrix = glm::toMat4(transform.rot);
			angle += 0.000001f * EngineWrapper::dt;
			modelMatrix = glm::rotate(modelMatrix, glm::radians(angle), glm::vec3(0.0f, 1.0f, 0.0f));

			modelMatrix = glm::translate(modelMatrix, transform.pos);
			modelMatrix = glm::scale(modelMatrix, transform.scale);

			bgfx::setTransform(&modelMatrix[0][0]);
		
			bgfx::setVertexBuffer(0, meshAsset.lock()->vbuf);
			bgfx::setIndexBuffer(meshAsset.lock()->ibuf);

			glm::mat3 normalMatrix = glm::transpose(glm::inverse(glm::mat3(modelMatrix)));

			bgfx::setUniform(EngineWrapper::shaderUniforms.at("normalMatrix"), &normalMatrix[0]);

			// render diffuse map
			std::weak_ptr<AssetLibrary::Texture> diffTexAsset;
			if (EngineWrapper::assetLib.getTexture(material.diffuse_tex, diffTexAsset))
			{
				bgfx::setTexture(0,
					diffTexAsset.lock()->sampler,
					diffTexAsset.lock()->texHandle);
			}

			// render normal map
			std::weak_ptr<AssetLibrary::Texture> normTexAsset;
			if (EngineWrapper::assetLib.getTexture(material.normal_tex, normTexAsset))
			{
				bgfx::setTexture(1,
					normTexAsset.lock()->sampler,
					normTexAsset.lock()->texHandle);
			}

			bgfx::setState(state);

			bgfx::submit(kRenderPassGeometry, shader.program);
		}
	}
}
