#pragma once
#include <bgfx/bgfx.h>
#include <vector>
#include <unordered_map>
#include <spdlog/spdlog.h>
#include <filesystem>
#include <fstream>
#include <bimg/bimg.h>
#include <glm/vec3.hpp>
#include <glm/glm.hpp>
#include <string>
#include <regex>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "RenderCommon.h"

namespace fs = std::filesystem;

namespace SolsticeGE {

	/// <summary>
	/// The asset library contains data
	/// that will be availible through
	/// the entire lifecycle of the game.
	/// 
	/// In the future there might be some sort
	/// of asset streaming system where this isn't
	/// the case
	/// </summary>
	class AssetLibrary
	{
	public:
		
		AssetLibrary();

		// asset types
		// V==================V

		struct Scene {
			std::vector<ASSET_ID> meshes;
			std::vector<ASSET_ID> materials;
			std::vector<ASSET_ID> textures;
		};

		struct Mesh {

			// meshes can be "loaded"
			// without actually being copied
			// to the GPU, this flag lets us know
			// the data is on the GPU and ready to be rendered
			bool bufferLoaded;

			bgfx::VertexBufferHandle vbuf;
			bgfx::IndexBufferHandle ibuf;

			// the material of the mesh
			ASSET_ID material;

			std::vector<BasicVertex> vdata;
			std::vector<uint16_t> idata;
		};
		
		struct Texture {

			bool bufferLoaded;

			bgfx::TextureHandle texHandle;

			unsigned char* texData;
			float* texDataFloat;

			bgfx::TextureInfo texInfo;

			bgfx::UniformHandle sampler;
		};

		struct Material {
			ASSET_ID diffuse_tex;
			ASSET_ID normal_tex;
			ASSET_ID ao_tex;
			ASSET_ID metal_tex;
			ASSET_ID roughness_tex;
			ASSET_ID emissive_tex;

			bool isPacked;
		};

		bool getMesh(const ASSET_ID& id, std::weak_ptr<Mesh>& mesh);
		bool getTexture(const ASSET_ID& id, std::weak_ptr<Texture>& texture);
		bool getCubemap(const ASSET_ID& id, std::weak_ptr<Texture>& texture);
		bool getMaterial(const ASSET_ID& id, std::weak_ptr<Material>& material);
		bool getScene(const std::string& name, std::weak_ptr<Scene>& scene);

		std::unordered_map<ASSET_ID, std::shared_ptr<Texture>> getCubemaps();

		bool loadAssets(const std::string& assetDir);
		
	private:

		ASSET_ID m_meshCount;
		ASSET_ID m_textureCount;
		ASSET_ID m_materialCount;
		ASSET_ID m_cubemapCount;

		std::string m_assetsRoot;

		void loadScene(const fs::path& fileName);
		ASSET_ID loadMesh(aiMesh* inMesh, std::shared_ptr<Scene>& scene);
		ASSET_ID loadTexture2D(aiTexture* inTex);
		ASSET_ID loadTexture2D(const fs::path& fileName);
		ASSET_ID loadTextureCube(const fs::path& fileName);
		ASSET_ID loadMaterial(aiMaterial* inMat, std::shared_ptr<Scene>& scene, fs::path sceneDir);

		std::unordered_map<ASSET_ID, std::shared_ptr<Mesh>> mp_meshes;
		std::unordered_map<ASSET_ID, std::shared_ptr<Texture>> mp_textures;
		std::unordered_map<ASSET_ID, std::shared_ptr<Texture>> mp_cubemaps;
		std::unordered_map<ASSET_ID, std::shared_ptr<Material>> mp_materials;

		// string map for easy use
		std::unordered_map<std::string, std::shared_ptr<Scene>> mp_scenes;

	};
}

