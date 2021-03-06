#include "MeshRenderSystem.h"
#include "EngineWrapper.h"

using namespace SolsticeGE;

MeshRenderSystem::MeshRenderSystem()
{
}

void MeshRenderSystem::update(entt::registry& registry)
{
	auto mesh_view = registry.view<
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

	for (const auto& [entity, transform, mesh, shader, material] : mesh_view.each())
	{
		std::weak_ptr<AssetLibrary::Mesh> meshAsset;
		if (!EngineWrapper::assetLib.getMesh(mesh.assetId, meshAsset))
		{
			continue;
		}

		if (meshAsset.lock() != nullptr && meshAsset.lock()->bufferLoaded) {

			bgfx::setTransform(&transform.computedMatrix[0][0]);
		
			bgfx::setVertexBuffer(0, meshAsset.lock()->vbuf);
			bgfx::setIndexBuffer(meshAsset.lock()->ibuf);

			glm::mat3 normalMatrix = glm::transpose(glm::inverse(glm::mat3(transform.computedMatrix)));
			bgfx::setUniform(EngineWrapper::shaderUniforms.at("normalMatrix"), &normalMatrix[0]);

			// render diffuse map
			if (material.diffuse_tex != ASSET_ID_INVALID)
				setTexture(material.diffuse_tex, 0);

			// render normal map
			if (material.normal_tex != ASSET_ID_INVALID)
				setTexture(material.normal_tex, 1);

			bgfx::setUniform(EngineWrapper::shaderUniforms.at("isPacked"), 
				&glm::vec4(material.isPacked ? 1.0f : 0.0f, 0.0f, 0.0f, 0.0f)[0]);

			// render ao map
			if (material.ao_tex != ASSET_ID_INVALID)
				setTexture(material.ao_tex, 2);

			// render metalness map
			if (material.metal_tex != ASSET_ID_INVALID)
				setTexture(material.metal_tex, 3);

			// render roughness map
			if (material.roughness_tex != ASSET_ID_INVALID)
				setTexture(material.roughness_tex, 4);

			// render emissive map
			if (material.emissive_tex != ASSET_ID_INVALID)
				setTexture(material.emissive_tex, 5);

			bgfx::setState(state);

			bgfx::submit(kRenderPassGeometry, shader.program);
		}
	}
}

void MeshRenderSystem::setTexture(const ASSET_ID& texture, int shaderSlot)
{
	std::weak_ptr<AssetLibrary::Texture> texAsset;
	if (EngineWrapper::assetLib.getTexture(texture, texAsset))
	{
		bgfx::setTexture(shaderSlot,
			texAsset.lock()->sampler,
			texAsset.lock()->texHandle);
	}
}
