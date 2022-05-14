#include "AssetLibrary.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

using namespace SolsticeGE;
namespace fs = std::filesystem;

AssetLibrary::AssetLibrary()
{
	this->m_meshCount = 0;
	this->m_textureCount = 0;
}

bool AssetLibrary::getMesh(const ASSET_ID& id, std::weak_ptr<Mesh>& mesh)
{
	const auto& iter = this->mp_meshes.find(id);
	if (iter != this->mp_meshes.end())
	{
		if (iter->second != nullptr)
		{
			mesh = iter->second;
			return true;
		}
		else {
			spdlog::error("Mesh: {} was loaded but is now null!", id);
			return false;
		}
	}
	else {
		spdlog::error("Mesh: {} is not loaded!", id);
		return false;
	}
}

bool AssetLibrary::getTexture(const ASSET_ID& id, std::weak_ptr<Texture>& texture)
{
	const auto& iter = this->mp_textures.find(id);
	if (iter != this->mp_textures.end())
	{
		if (iter->second != nullptr)
		{
			texture = iter->second;
			return true;
		}
		else {
			spdlog::error("Texture: {} was loaded but is now null!", id);
			return false;
		}
	}
	else {
		spdlog::error("Texture: {} is not loaded!", id);
		return false;
	}
}

bool AssetLibrary::getMaterial(const ASSET_ID& id, std::weak_ptr<Material>& material)
{
	const auto& iter = this->mp_materials.find(id);
	if (iter != this->mp_materials.end())
	{
		if (iter->second != nullptr)
		{
			material = iter->second;
			return true;
		}
		else {
			spdlog::error("Material: {} was loaded but is now null!", id);
			return false;
		}
	}
	else {
		spdlog::error("Material: {} is not loaded!", id);
		return false;
	}
}

bool AssetLibrary::getScene(const std::string& name, std::weak_ptr<Scene>& scene)
{
	const auto& iter = this->mp_scenes.find(name);
	if (iter != this->mp_scenes.end())
	{
		if (iter->second != nullptr)
		{
			scene = iter->second;
			return true;
		}
		else {
			spdlog::error("Scene: {} was loaded but is now null!", name);
			return false;
		}
	}
	else {
		spdlog::error("Scene: {} is not loaded!", name);
		return false;
	}
}

bool AssetLibrary::loadAssets(const std::string& assetDir)
{
	const fs::path assetPath(assetDir);

	for (const auto& entry : fs::directory_iterator(assetPath)) {
		const auto filenameStr = entry.path().filename().string();
		if (entry.is_regular_file()) {
			if (entry.path().extension() == ".glb" ||
				entry.path().extension() == ".gltf" ||
				entry.path().extension() == ".fbx")
			{
				// load meshes (with packed textures)
				loadScene(entry.path().string());
			}

			//// load cubemaps
			//if (entry.path().extension() == ".hdr")
			//{
			//	loadTextureCube(entry.path().string());
			//}

			// etc...
		}
	}

	return true;
}

/// <summary>
/// Loads a scene, this
/// could contain a whole world with
/// meshes, textures, materials and lights
/// </summary>
/// <param name="fileName"></param>
void AssetLibrary::loadScene(const std::string& fileName)
{
	spdlog::info("Loading scene from {}", fileName);

	Assimp::Importer importer;

	const aiScene* inScene = importer.ReadFile(fileName,
		aiProcess_Triangulate |
		aiProcess_OptimizeGraph |
		aiProcess_OptimizeMeshes |
		aiProcess_GenUVCoords |
		aiProcess_GenSmoothNormals |
		aiProcess_CalcTangentSpace |
		aiProcess_SortByPType |
		aiProcess_JoinIdenticalVertices);

	// If the import failed, report it
	if (inScene == nullptr) {
		spdlog::error("Scene import failed! Error: {}", importer.GetErrorString());
		return;
	}

	std::shared_ptr<AssetLibrary::Scene> scene = std::make_shared<AssetLibrary::Scene>();

	// load scene textures
	if (inScene != nullptr && inScene->HasTextures()) {

		for (size_t i = 0; i < inScene->mNumTextures; i++)
		{
			aiTexture* inTex = inScene->mTextures[i];

			ASSET_ID id = loadTexture2D(inTex);
			scene->textures.push_back(id);
		}
	}

	// load scene materials
	if (inScene != nullptr && inScene->HasMaterials()) {

		for (size_t i = 0; i < inScene->mNumMaterials; i++)
		{
			aiMaterial* inMat = inScene->mMaterials[i];

			ASSET_ID id = loadMaterial(inMat, scene);
			scene->materials.push_back(id);
		}
	}

	// load scene meshes
	if (inScene != nullptr && inScene->HasMeshes()) {

		for (size_t i = 0; i < inScene->mNumMeshes; i++)
		{
			aiMesh* inMesh = inScene->mMeshes[i];
			ASSET_ID id = loadMesh(inMesh, scene);
			scene->meshes.push_back(id);
		}
	}

	this->mp_scenes.emplace(fileName, scene);

	spdlog::info("Done loading scene from {}", fileName);
}

ASSET_ID AssetLibrary::loadMaterial(aiMaterial* inMat, std::shared_ptr<Scene>& scene)
{
	std::shared_ptr<Material> material = std::make_shared<Material>();
	material->diffuse_tex = ASSET_ID_INVALID;
	material->normal_tex = ASSET_ID_INVALID;
	material->ao_tex = ASSET_ID_INVALID;
	material->metal_tex = ASSET_ID_INVALID;
	material->roughness_tex = ASSET_ID_INVALID;
	material->emissive_tex = ASSET_ID_INVALID;

	material->isPacked = false;

	// The regular expression to find the index of the texture if it's an
	// embedded texture.
	std::regex regexExpress("^\\*\\d+");
	std::cmatch regexMatches;

	auto getMatTextureLambda = [&](const aiString texturePath) {
		// Check if it's an embedded or external  texture.
		if (std::regex_search(texturePath.C_Str(), regexMatches, regexExpress))
		{
			// Get the index str.
			std::string indexStr = *(regexMatches.begin());

			// Drop the "*" character.
			indexStr = indexStr.erase(0, 1);

			// Convert the string to an integer. (This is the index in the
			// Scene::mTextures[] array.
			int index = std::stoi(indexStr);

			spdlog::info("Texture embedded, loading from index {}", index);
			return scene->textures[index];
		}
		else
		{
			// file based texture (external)
			spdlog::info("Texture external, loading from file {}", texturePath.C_Str());
			return loadTexture2D(texturePath.C_Str());
		}
	};

	// Get the path to the texture.
	aiString texturePath;
	if (inMat->GetTexture(aiTextureType_DIFFUSE, 0, &texturePath) ==
		aiReturn_SUCCESS)
	{
		material->diffuse_tex = getMatTextureLambda(texturePath);
		spdlog::info("[Mat]: Diffuse: {}", material->diffuse_tex);
	}

	if (inMat->GetTexture(aiTextureType_NORMALS, 0, &texturePath) ==
		aiReturn_SUCCESS)
	{
		material->normal_tex = getMatTextureLambda(texturePath);
		spdlog::info("[Mat]: Normal: {}", material->normal_tex);

	}

	if (inMat->GetTexture(aiTextureType_AMBIENT_OCCLUSION, 0, &texturePath) ==
		aiReturn_SUCCESS)
	{
		material->ao_tex = getMatTextureLambda(texturePath);
		spdlog::info("[Mat]: Ambient Occlusion: {}", material->ao_tex);

	}

	if (inMat->GetTexture(aiTextureType_METALNESS, 0, &texturePath) ==
		aiReturn_SUCCESS)
	{
		material->metal_tex = getMatTextureLambda(texturePath);
		spdlog::info("[Mat]: Metalness: {}", material->metal_tex);

	}

	if (inMat->GetTexture(aiTextureType_DIFFUSE_ROUGHNESS, 0, &texturePath) ==
		aiReturn_SUCCESS)
	{
		material->roughness_tex = getMatTextureLambda(texturePath);
		spdlog::info("[Mat]: Roughness: {}", material->roughness_tex);

	}

	// we're just gonna assume unknown textures are packed
	// AO Metal Roughness -> RGB
	if (inMat->GetTexture(aiTextureType_UNKNOWN, 0, &texturePath) == aiReturn_SUCCESS
		&& material->ao_tex == ASSET_ID_INVALID
		&& material->metal_tex == ASSET_ID_INVALID
		&& material->roughness_tex == ASSET_ID_INVALID)
	{
		material->isPacked = true;
		material->ao_tex = getMatTextureLambda(texturePath);
		spdlog::info("[Mat]: Unknown (Assuming Packed): {}", material->ao_tex);
	}

	if (inMat->GetTexture(aiTextureType_EMISSIVE, 0, &texturePath) ==
		aiReturn_SUCCESS)
	{
		material->emissive_tex = getMatTextureLambda(texturePath);
		spdlog::info("[Mat]: Emissive: {}", material->emissive_tex);

	}

	ASSET_ID idOut = this->m_materialCount;
	this->mp_materials.emplace(this->m_materialCount, material);
	this->m_materialCount++;

	return idOut;
}

/// <summary>
/// Loads a mesh from an assimp mesh
/// </summary>
/// <param name="inMesh"></param>
ASSET_ID AssetLibrary::loadMesh(aiMesh* inMesh, std::shared_ptr<Scene>& scene)
{
	std::shared_ptr<Mesh> mesh = std::make_shared<Mesh>();
	mesh->bufferLoaded = false;

	ASSET_ID idOut = UINT16_MAX;

	if (inMesh->mPrimitiveTypes & aiPrimitiveType_TRIANGLE && inMesh->HasPositions() && inMesh->HasFaces())
	{
		mesh->material = scene->materials[inMesh->mMaterialIndex];
		mesh->vdata.reserve(mesh->vdata.size() + (inMesh->mNumVertices));
		for (size_t v = 0; v < inMesh->mNumVertices; v++)
		{
			aiVector3D pos = inMesh->mVertices[v];

			BasicVertex vert = {
				pos.x, pos.y, pos.z,
				0.0f, 0.0f, 1.0f,
				1.0f, 0.0f, 0.0f,
				0.0f, 1.0f, 0.0f,
				1.0f, 1.0f,
				0xff0000ff
			};

			if (inMesh->HasTangentsAndBitangents())
			{
				aiVector3D tangent = inMesh->mTangents[v];
				vert.m_tanx = tangent.x;
				vert.m_tany = tangent.y;
				vert.m_tanz = tangent.z;

				aiVector3D bitangent = inMesh->mBitangents[v];
				vert.m_btanx = bitangent.x;
				vert.m_btany = bitangent.y;
				vert.m_btanz = bitangent.z;
			}

			if (inMesh->HasNormals())
			{
				aiVector3D normal = inMesh->mNormals[v];
				vert.m_normx = normal.x;
				vert.m_normy = normal.y;
				vert.m_normz = normal.z;
			}

			if (inMesh->HasTextureCoords(0))
			{
				aiVector3D uv = inMesh->mTextureCoords[0][v];
				vert.m_u = uv.x;
				vert.m_v = 1 - uv.y;
			}

			// fix for symmetric meshes
			glm::vec3 normal(vert.m_normx, vert.m_normy, vert.m_normz);
			glm::vec3 tangent(vert.m_tanx, vert.m_tany, vert.m_tanz);
			glm::vec3 bitangent(vert.m_btanx, vert.m_btany, vert.m_btanz);

			if (glm::dot(glm::cross(normal, tangent), bitangent) < 0.0f) {
				tangent = tangent * -1.0f;

				vert.m_tanx = tangent.x;
				vert.m_tany = tangent.y;
				vert.m_tanz = tangent.z;
			}

			mesh->vdata.push_back(vert);
		}

		mesh->idata.reserve(mesh->idata.size() + (inMesh->mNumFaces * 3));
		for (size_t f = 0; f < inMesh->mNumFaces; f++)
		{
			aiFace& face = inMesh->mFaces[f];
			if (face.mNumIndices == 3)
			{
				mesh->idata.push_back(static_cast<uint16_t>(face.mIndices[0]));
				mesh->idata.push_back(static_cast<uint16_t>(face.mIndices[1]));
				mesh->idata.push_back(static_cast<uint16_t>(face.mIndices[2]));
			}
		}

		spdlog::info("Model loaded from {}, N(verts): {} N(idx): {}", inMesh->mName.C_Str(), mesh->vdata.size(), mesh->idata.size());

		idOut = this->m_meshCount;
		this->mp_meshes.emplace(this->m_meshCount, mesh);
		this->m_meshCount++;
	}

	return idOut;
}

/// <summary>
/// Loads a 2d texture from memory
/// </summary>
/// <param name="inTex"></param>
/// <returns></returns>
ASSET_ID AssetLibrary::loadTexture2D(aiTexture* inTex)
{
	std::shared_ptr<Texture> texture = std::make_shared<Texture>();
	texture->bufferLoaded = false;

	int width = 0, height = 0, nrComponents = 0;
	texture->texData = stbi_load_from_memory(
		reinterpret_cast<unsigned char*>(inTex->pcData),
		inTex->mWidth, &width, &height, &nrComponents, STBI_rgb_alpha);

	bgfx::TextureInfo texInfo;

	texInfo.width = width;
	texInfo.height = height;
	texInfo.storageSize = (width * height) * 4;
	texInfo.format = bgfx::TextureFormat::RGBA8;
	texInfo.cubeMap = false;

	texture->texInfo = texInfo;

	ASSET_ID idOut = this->m_textureCount;
	this->mp_textures.emplace(this->m_textureCount, texture);
	this->m_textureCount++;

	return idOut;
}

/// <summary>
/// Loads a 2d texture from file
/// </summary>
/// <param name="fileName"></param>
/// <returns></returns>
ASSET_ID AssetLibrary::loadTexture2D(const std::string& fileName)
{
	std::shared_ptr<Texture> texture = std::make_shared<Texture>();
	texture->bufferLoaded = false;

	std::string filePath = "assets/" + fileName;

	int width = 0, height = 0, nrComponents = 0;
	unsigned char* data = stbi_load(filePath.c_str(), &width, &height, &nrComponents, STBI_rgb_alpha);

	if (height != 0 && width != 0) {
		texture->texData = data;

		bgfx::TextureInfo texInfo;

		texInfo.width = width;
		texInfo.height = height;
		texInfo.storageSize = (width * height) * 4;
		texInfo.format = bgfx::TextureFormat::RGBA8;
		texInfo.cubeMap = false;

		texture->texInfo = texInfo;

		spdlog::info("Texture loaded from {}", fileName);

		ASSET_ID idOut = this->m_textureCount;
		this->mp_textures.emplace(this->m_textureCount, texture);
		this->m_textureCount++;

		return idOut;
	}
	else {
		return ASSET_ID_INVALID;
	}
}

/// <summary>
/// Loads a cubemap texture from file
/// </summary>
/// <param name="fileName"></param>
/// <returns></returns>
ASSET_ID AssetLibrary::loadTextureCube(const std::string& fileName)
{
	std::shared_ptr<Texture> texture = std::make_shared<Texture>();
	texture->bufferLoaded = false;

	int width, height, nrComponents;
	float* data = stbi_loadf(fileName.c_str(), &width, &height, &nrComponents, STBI_rgb_alpha);

	texture->texDataFloat = data;

	bgfx::TextureInfo texInfo;

	texInfo.width = width;
	texInfo.height = height;
	texInfo.storageSize = (width * height) * 4;
	texInfo.format = bgfx::TextureFormat::RGBA16F;
	texInfo.cubeMap = false;

	texture->texInfo = texInfo;

	ASSET_ID idOut = this->m_textureCount;
	this->mp_textures.emplace(this->m_textureCount, texture);
	this->m_textureCount++;

	spdlog::info("Texture loaded from {} with channels {}", fileName, nrComponents);

	return idOut;
}
