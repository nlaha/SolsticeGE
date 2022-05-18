#include "SceneHierarchySystem.h"
#include "EngineWrapper.h"

using namespace SolsticeGE;

void SceneHierarchySystem::update(entt::registry& registry)
{
	auto transform_view = registry.view<
		c_transform
	>();

	for (auto& entity : transform_view)
	{
		auto& transform = transform_view.get<c_transform>(entity);

		transform.computedMatrix = glm::identity<glm::mat4>();
		transform.computedMatrix = glm::translate(transform.computedMatrix, transform.pos);
		transform.computedMatrix = transform.computedMatrix * glm::toMat4(transform.rot);
		transform.computedMatrix = glm::scale(transform.computedMatrix, transform.scale);
	}

	auto parent_view = registry.view<
		c_transform,
		c_parent
	>();

	for (auto& entity : parent_view)
	{
		auto& transform = parent_view.get<c_transform>(entity);
		const auto& parent = parent_view.get<c_parent>(entity);

		const auto& parent_transform = transform_view.get<c_transform>(parent.parent);
		transform.computedMatrix = parent_transform.computedMatrix * transform.computedMatrix;
	}
}
