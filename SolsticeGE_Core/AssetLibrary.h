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

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "RenderCommon.h"

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

		struct Mesh {

			// meshes can be "loaded"
			// without actually being copied
			// to the GPU, this flag lets us know
			// the data is on the GPU and ready to be rendered
			bool bufferLoaded;

			bgfx::VertexBufferHandle vbuf;
			bgfx::IndexBufferHandle ibuf;

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

		bool getMesh(const std::string& name, std::weak_ptr<Mesh>& mesh);
		bool getTexture(const std::string& name, std::weak_ptr<Texture>& texture);

		bool loadAssets(const std::string& assetDir);
		
	private:

		std::uint16_t m_meshCount;
		std::uint16_t m_textureCount;

		std::string m_assetsRoot;

		std::weak_ptr<Mesh> loadMesh(const std::string& fileName);
		std::weak_ptr<Texture> loadTexture2D(const std::string& fileName);
		std::weak_ptr<Texture> loadTextureCube(const std::string& fileName);

		std::unordered_map<std::string, std::shared_ptr<Mesh>> mp_meshes;
		std::unordered_map<std::string, std::shared_ptr<Texture>> mp_textures;

	};
}

