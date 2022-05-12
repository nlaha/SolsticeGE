#pragma once
#include <entt/entt.hpp>
#include <bgfx/bgfx.h>
#include <glm/matrix.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/quaternion.hpp>

#include "System.h"
#include "RenderComponents.h"

namespace SolsticeGE {

	/// <summary>
	/// Mesh render system
	/// responsible for rendering entities
	/// that have c_mesh, c_shader and c_transform components
	/// </summary>
	class MeshRenderSystem : public System
	{
	public:
		MeshRenderSystem();

		void update(entt::registry& registry);

	private:

		float angle;
	};

}
