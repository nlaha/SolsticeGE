#include "ModelLoaderSystem.h"

using namespace SolsticeGE;

void ModelLoaderSystem::update(entt::registry& registry)
{
	auto ecs_view = registry.view<
		c_mesh,
		c_model
	>();

	Assimp::Importer importer;

	for (const auto& entity : ecs_view)
	{
		auto& model = ecs_view.get<c_model>(entity);
		auto& mesh = ecs_view.get<c_mesh>(entity);

		if (!mesh.isValid) {

			const aiScene* scene = importer.ReadFile(model.filepath,
				aiProcess_Triangulate |
				aiProcess_OptimizeGraph |
				aiProcess_OptimizeMeshes |
				aiProcess_GenUVCoords |
				aiProcess_CalcTangentSpace |
				aiProcess_GenSmoothNormals | 
				aiProcess_SortByPType |
				aiProcess_CalcTangentSpace |
				aiProcess_JoinIdenticalVertices);

			// If the import failed, report it
			if (scene == nullptr) {
				spdlog::error("Model import failed! Error: {}", importer.GetErrorString());
				return;
			}

			if (scene != nullptr && scene->HasMeshes()) {

				for (size_t i = 0; i < scene->mNumMeshes; i++)
				{
					aiMesh* inMesh = scene->mMeshes[i];

					if (inMesh->mPrimitiveTypes & aiPrimitiveType_TRIANGLE && inMesh->HasPositions() && inMesh->HasFaces())
					{
						mesh.vdata.reserve(mesh.vdata.size() + (inMesh->mNumVertices));
						for (size_t v = 0; v < inMesh->mNumVertices; v++)
						{
							aiVector3D pos = inMesh->mVertices[v];

							BasicVertex vert = {
								pos.x, pos.y, pos.z,
								1.0f, 1.0f, 1.0f,
								1.0f, 1.0f,
								0xff0000ff
							};

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
								vert.m_v = uv.y;
							}

							mesh.vdata.push_back(vert);
						}

						mesh.idata.reserve(mesh.idata.size() + (inMesh->mNumFaces * 3));
						for (size_t f = 0; f < inMesh->mNumFaces; f++)
						{
							aiFace& face = inMesh->mFaces[f];
							if (face.mNumIndices == 3) 
							{
								mesh.idata.push_back(static_cast<uint16_t>(face.mIndices[0]));
								mesh.idata.push_back(static_cast<uint16_t>(face.mIndices[1]));
								mesh.idata.push_back(static_cast<uint16_t>(face.mIndices[2]));
							}
						}
					}
				}

				BasicVertex::init();

				// Create static vertex buffer.
				mesh.vbuf = bgfx::createVertexBuffer(
					// Static data can be passed with bgfx::makeRef
					//bgfx::makeRef(s_cubeVertices, sizeof(s_cubeVertices))
					bgfx::makeRef(mesh.vdata.data(), mesh.vdata.size() * sizeof(mesh.vdata[0]))
					, BasicVertex::ms_layout
				);

				// Create static index buffer for triangle list rendering.
				mesh.ibuf = bgfx::createIndexBuffer(
					// Static data can be passed with bgfx::makeRef
					//bgfx::makeRef(s_cubeTriList, sizeof(s_cubeTriList))
					bgfx::makeRef(mesh.idata.data(), mesh.idata.size() * sizeof(mesh.idata[0]))
				);

				spdlog::info("Model loaded from {}, N(verts): {} N(idx): {}", model.filepath, mesh.vdata.size(), mesh.idata.size());

				mesh.isValid = true;

			}
		}
	}
}
