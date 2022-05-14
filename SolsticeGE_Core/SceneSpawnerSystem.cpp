#include "SceneSpawnerSystem.h"
#include "EngineWrapper.h"

using namespace SolsticeGE;

void SceneSpawnerSystem::update(entt::registry& registry)
{
	auto ecs_view = registry.view<
		c_scene,
		c_transform
	>();

	for (const auto& entity : ecs_view)
	{
		auto& scene = ecs_view.get<c_scene>(entity);
		const auto& transform = ecs_view.get<c_transform>(entity);

		/* TODO: Parent entities somehow to the scene owner entity */
		if (!scene.isLoaded) {
			std::weak_ptr<AssetLibrary::Scene> sceneAsset;
			if (EngineWrapper::assetLib.getScene(scene.sceneName, sceneAsset))
			{
				for (const auto& mesh : sceneAsset.lock()->meshes)
				{
					std::weak_ptr<AssetLibrary::Mesh> meshAsset;
					if (EngineWrapper::assetLib.getMesh(mesh, meshAsset))
					{
						std::weak_ptr<AssetLibrary::Material> materialAsset;
						if (EngineWrapper::assetLib.getMaterial(meshAsset.lock()->material, materialAsset))
						{
							spdlog::info("Spawning entity for mesh {}", mesh);
							// spawn ecs entities for each mesh
							const auto entity = registry.create();
							registry.emplace<c_mesh>(entity, mesh);
							registry.emplace<c_transform>(entity,
								transform.pos,
								transform.rot,
								transform.scale); // todo add per mesh offset
							registry.emplace<c_shader>(entity,
								EngineWrapper::vs_mesh,
								EngineWrapper::fs_mesh,
								EngineWrapper::prog_mesh);
							registry.emplace<c_material>(entity,
								materialAsset.lock()->diffuse_tex,
								materialAsset.lock()->normal_tex,
								materialAsset.lock()->ao_tex,
								materialAsset.lock()->metal_tex,
								materialAsset.lock()->roughness_tex,
								materialAsset.lock()->emissive_tex,
								materialAsset.lock()->isPacked
							);
						}
					}
				}
			}

			scene.isLoaded = true;
		}
	}
}
