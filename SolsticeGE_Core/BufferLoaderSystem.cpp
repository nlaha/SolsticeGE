#include "BufferLoaderSystem.h"
#include "EngineWrapper.h"

using namespace SolsticeGE;

void BufferLoaderSystem::update(entt::registry& registry)
{
	auto mesh_view = registry.view<
		c_mesh
	>();

	auto material_view = registry.view<
		c_material
	>();

	for (const auto& entity : mesh_view)
	{
		auto& mesh = mesh_view.get<c_mesh>(entity);

		std::weak_ptr<AssetLibrary::Mesh> meshAsset;
		if (!EngineWrapper::assetLib.getMesh(mesh.assetId, meshAsset))
		{
			continue;
		}

		if (!meshAsset.lock()->bufferLoaded) {
			// Create static vertex buffer.
			meshAsset.lock()->vbuf = bgfx::createVertexBuffer(
				// Static data can be passed with bgfx::makeRef
				//bgfx::makeRef(s_cubeVertices, sizeof(s_cubeVertices))
				bgfx::makeRef(meshAsset.lock()->vdata.data(), meshAsset.lock()->vdata.size() * sizeof(meshAsset.lock()->vdata[0]))
				, BasicVertex::ms_layout
			);

			// Create static index buffer for triangle list rendering.
			meshAsset.lock()->ibuf = bgfx::createIndexBuffer(
				// Static data can be passed with bgfx::makeRef
				//bgfx::makeRef(s_cubeTriList, sizeof(s_cubeTriList))
				bgfx::makeRef(meshAsset.lock()->idata.data(), meshAsset.lock()->idata.size() * sizeof(meshAsset.lock()->idata[0]))
			);

			meshAsset.lock()->bufferLoaded = true;
		}
	}

	for (const auto& entity : material_view)
	{
		auto& material = material_view.get<c_material>(entity);

		// load diffuse texture
		std::weak_ptr<AssetLibrary::Texture> diffTexAsset;
		if (!EngineWrapper::assetLib.getTexture(material.diffuse_tex, diffTexAsset))
		{
			continue;
		}

		moveTextureToGPU(diffTexAsset, material.diffuse_tex);

		// load normal texture
		std::weak_ptr<AssetLibrary::Texture> normTexAsset;
		if (!EngineWrapper::assetLib.getTexture(material.normal_tex, normTexAsset))
		{
			continue;
		}

		moveTextureToGPU(normTexAsset, material.normal_tex);
	}
}

void BufferLoaderSystem::moveTextureToGPU(const std::weak_ptr<AssetLibrary::Texture>& texture, const std::uint16_t& id)
{
	if (!texture.lock()->bufferLoaded) {

		std::stringstream samplerName;
		samplerName << "sampler" << id;

		texture.lock()->sampler = bgfx::createUniform(
			samplerName.str().c_str(),
			bgfx::UniformType::Sampler);

		const bgfx::Memory* mem = bgfx::makeRef(
			texture.lock()->texData.data(), texture.lock()->texData.size()
		);

		texture.lock()->texHandle = bgfx::createTexture(
			mem,
			BGFX_TEXTURE_NONE,
			0, &texture.lock()->texInfo);

		texture.lock()->bufferLoaded = true;
	}
}
