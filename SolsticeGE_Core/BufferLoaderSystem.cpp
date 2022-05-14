#include "BufferLoaderSystem.h"
#include "EngineWrapper.h"
#include "stb_image.h"

using namespace SolsticeGE;

BufferLoaderSystem::BufferLoaderSystem()
{
	this->m_texCount = 0;
}

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
			spdlog::info("Loading GPU data for mesh {}", mesh.assetId);
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
		if (material.diffuse_tex != ASSET_ID_INVALID)
			moveTextureToGPU(material.diffuse_tex);

		// load normal texture
		if (material.normal_tex != ASSET_ID_INVALID)
			moveTextureToGPU(material.normal_tex);

		// load ao texture
		if (material.ao_tex != ASSET_ID_INVALID)
			moveTextureToGPU(material.ao_tex);

		// load metalness texture
		if (material.metal_tex != ASSET_ID_INVALID)
			moveTextureToGPU(material.metal_tex);

		// load roughness texture
		if (material.roughness_tex != ASSET_ID_INVALID)
			moveTextureToGPU(material.roughness_tex);

		// load emissive texture
		if (material.emissive_tex != ASSET_ID_INVALID)
			moveTextureToGPU(material.emissive_tex);
	}
}

static void imageReleaseFunction(void* ptr)
{
	stbi_image_free(ptr);
}

void BufferLoaderSystem::moveTextureToGPU(const ASSET_ID& texture)
{

	std::weak_ptr<AssetLibrary::Texture> texAsset;
	if (EngineWrapper::assetLib.getTexture(texture, texAsset))
	{
		if (!texAsset.lock()->bufferLoaded) {

			std::stringstream samplerName;
			samplerName << "sampler" << m_texCount;
			m_texCount++;

			texAsset.lock()->sampler = bgfx::createUniform(
				samplerName.str().c_str(),
				bgfx::UniformType::Sampler);

			bgfx::TextureInfo texInfo = texAsset.lock()->texInfo;

			// load cubemaps
			if (texInfo.cubeMap) {
				const bgfx::Memory* mem = bgfx::makeRef(
					texAsset.lock()->texDataFloat, texInfo.storageSize,
					(bgfx::ReleaseFn)imageReleaseFunction
				);

				texAsset.lock()->texHandle = bgfx::createTextureCube(
					texInfo.width, false, 1, 
					texInfo.format, BGFX_TEXTURE_NONE, mem);
			}
			// load 2d textures
			else {
				const bgfx::Memory* mem = bgfx::makeRef(
					texAsset.lock()->texData, texInfo.storageSize,
					(bgfx::ReleaseFn)imageReleaseFunction
				);

				texAsset.lock()->texHandle = bgfx::createTexture2D(
					texInfo.height, texInfo.height,
					false, 1, texInfo.format, BGFX_TEXTURE_NONE, mem);
			}

			texAsset.lock()->bufferLoaded = true;
		}
	}
}
