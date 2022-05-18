#pragma once

#include <bgfx/bgfx.h>
#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtc/quaternion.hpp>
#include <entt/entt.hpp>
#include <vector>

#include "AssetLibrary.h"
#include "RenderCommon.h"

namespace SolsticeGE {

	// TODO: move to specific file
	class Connection {
		// ip, health, etc...
	};

	/// <summary>
	/// Represents something with
	/// ownership in the network,
	/// for example, a projectile will
	/// be owned by the connection that shot it
	/// </summary>
	struct c_networked {
		Connection writeOwner;
	};

	struct c_parent {
		entt::entity parent;
	};

	struct c_player {

	};
}