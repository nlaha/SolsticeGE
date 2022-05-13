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

bool AssetLibrary::getMesh(const std::string& name, std::weak_ptr<Mesh>& mesh)
{
	const auto& iter = this->mp_meshes.find(name);
	if (iter != this->mp_meshes.end())
	{
		if (iter->second != nullptr)
		{
			mesh = iter->second;
			return true;
		}
		else {
			spdlog::error("Mesh: {} was loaded but is now null!", name);
			return false;
		}
	}
	else {
		spdlog::error("Mesh: {} is not loaded!", name);
		return false;
	}
}

bool AssetLibrary::getTexture(const std::string& name, std::weak_ptr<Texture>& texture)
{
	const auto& iter = this->mp_textures.find(name);
	if (iter != this->mp_textures.end())
	{
		if (iter->second != nullptr)
		{
			texture = iter->second;
			return true;
		}
		else {
			spdlog::error("Texture: {} was loaded but is now null!", name);
			return false;
		}
	}
	else {
		spdlog::error("Texture: {} is not loaded!", name);
		return false;
	}
}

bool AssetLibrary::loadAssets(const std::string& assetDir)
{
	const fs::path assetPath(assetDir);

	for (const auto& entry : fs::directory_iterator(assetPath)) {
		const auto filenameStr = entry.path().filename().string();
		if (entry.is_regular_file()) {
			if (entry.path().extension() == ".glb")
			{
				// load meshes (with packed textures)
				loadMesh(entry.path().string());
			}

			if (entry.path().extension() == ".jpg")
			{			
				loadTexture(entry.path().string());

				// load standalone textures
			}
			// load skyboxes

			// etc...
		}
	}

	return true;
}

std::weak_ptr<AssetLibrary::Mesh> AssetLibrary::loadMesh(const std::string& fileName)
{
	Assimp::Importer importer;
	std::shared_ptr<Mesh> mesh = std::make_shared<Mesh>();
	mesh->bufferLoaded = false;

	const aiScene* scene = importer.ReadFile(fileName,
		aiProcess_Triangulate |
		aiProcess_OptimizeGraph |
		aiProcess_OptimizeMeshes |
		aiProcess_GenUVCoords |
		aiProcess_GenSmoothNormals |
		aiProcess_CalcTangentSpace |
		aiProcess_SortByPType |
		aiProcess_JoinIdenticalVertices);

	// If the import failed, report it
	if (scene == nullptr) {
		spdlog::error("Model import failed! Error: {}", importer.GetErrorString());
		return mesh;
	}

	if (scene != nullptr && scene->HasMeshes()) {

		for (size_t i = 0; i < scene->mNumMeshes; i++)
		{
			aiMesh* inMesh = scene->mMeshes[i];

			if (inMesh->mPrimitiveTypes & aiPrimitiveType_TRIANGLE && inMesh->HasPositions() && inMesh->HasFaces())
			{
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
						vert.m_v = 1-uv.y;
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
			}
		}

		spdlog::info("Model loaded from {}, N(verts): {} N(idx): {}", fileName, mesh->vdata.size(), mesh->idata.size());

		BasicVertex::init();
	}

	this->mp_meshes.emplace(fileName, mesh);

	return mesh;
}

std::weak_ptr<AssetLibrary::Texture> AssetLibrary::loadTexture(const std::string& fileName)
{
	std::shared_ptr<Texture> texture = std::make_shared<Texture>();
	texture->bufferLoaded = false;

	//// open the file:
	//std::ifstream file(fileName, std::ios::binary);

	//// Stop eating new lines in binary mode!!!
	//file.unsetf(std::ios::skipws);

	//// get its size:
	//std::streampos fileSize;

	//file.seekg(0, std::ios::end);
	//fileSize = file.tellg();
	//file.seekg(0, std::ios::beg);

	//// reserve capacity
	//std::vector<unsigned char> vec;
	//vec.reserve(fileSize);

	//// read the data:
	//vec.insert(vec.begin(),
	//	std::istream_iterator<unsigned char>(file),
	//	std::istream_iterator<unsigned char>());

	int width, height, nrComponents;
	unsigned char* data = stbi_load(fileName.c_str(), &width, &height, &nrComponents, STBI_rgb_alpha);

	texture->texData = data;

	bgfx::TextureInfo texInfo;

	texInfo.width = width;
	texInfo.height = height;
	texInfo.storageSize = (width * height) * 4;
	texInfo.format = bgfx::TextureFormat::RGBA8;

	texture->texInfo = texInfo;

	this->mp_textures.emplace(fileName, texture);

	spdlog::info("Texture loaded from {} with channels {}", fileName, nrComponents);

	return texture;
}
