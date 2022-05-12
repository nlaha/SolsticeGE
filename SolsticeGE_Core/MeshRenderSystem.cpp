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

		if (meshAsset.lock() != nullptr && meshAsset.lock()->bufferLoaded) {

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
			setTexture(material.diffuse_tex, 0);

			// render normal map
			setTexture(material.normal_tex, 1);

			// render ao map
			setTexture(material.ao_tex, 2);

			// render metalness/roughness map
			setTexture(material.metalRoughness_tex, 3);

			// render emissive map
			setTexture(material.emissive_tex, 4);

			bgfx::setState(state);

			bgfx::submit(kRenderPassGeometry, shader.program);
		}
	}
}

void MeshRenderSystem::setTexture(const std::string& texture, int shaderSlot)
{
	std::weak_ptr<AssetLibrary::Texture> texAsset;
	if (EngineWrapper::assetLib.getTexture(texture, texAsset))
	{
		bgfx::setTexture(shaderSlot,
			texAsset.lock()->sampler,
			texAsset.lock()->texHandle);
	}
}
