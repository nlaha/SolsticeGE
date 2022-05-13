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
		moveTextureToGPU(material.diffuse_tex);

		// load normal texture
		moveTextureToGPU(material.normal_tex);

		// load ao texture
		moveTextureToGPU(material.ao_tex);

		// load metalness/roughness texture
		moveTextureToGPU(material.metalRoughness_tex);

		// load emissive texture
		moveTextureToGPU(material.emissive_tex);
	}
}

static int texCount = 0;
void BufferLoaderSystem::moveTextureToGPU(const std::string& texture)
{

	std::weak_ptr<AssetLibrary::Texture> texAsset;
	if (EngineWrapper::assetLib.getTexture(texture, texAsset))
	{
		if (!texAsset.lock()->bufferLoaded) {

			std::stringstream samplerName;
			samplerName << "sampler" << texCount;
			texCount++;

			texAsset.lock()->sampler = bgfx::createUniform(
				samplerName.str().c_str(),
				bgfx::UniformType::Sampler);

			bgfx::TextureInfo texInfo = texAsset.lock()->texInfo;

			// load cubemaps
			if (texInfo.cubeMap) {
				const bgfx::Memory* mem = bgfx::makeRef(
					texAsset.lock()->texDataFloat, texInfo.storageSize
				);

				texAsset.lock()->texHandle = bgfx::createTextureCube(
					texInfo.width, false, 1, 
					texInfo.format, BGFX_TEXTURE_NONE, mem);
			}
			// load 2d textures
			else {
				const bgfx::Memory* mem = bgfx::makeRef(
					texAsset.lock()->texData, texInfo.storageSize
				);

				texAsset.lock()->texHandle = bgfx::createTexture2D(
					texInfo.height, texInfo.height,
					false, 1, texInfo.format, BGFX_TEXTURE_NONE, mem);
			}

			texAsset.lock()->bufferLoaded = true;
		}
	}
}
