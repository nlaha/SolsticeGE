#pragma once
#include <glm/gtx/euler_angles.hpp>
#include <glm/gtx/quaternion.hpp>

namespace SolsticeGE {
	class Utility
	{
	public:
		inline static glm::quat eulerToQuat(float x, float y, float z)
		{
			glm::quat rotx = glm::quat(1.0f, 0.0f, 0.0f, x);
			glm::quat roty = glm::quat(0.0f, 1.0f, 0.0f, y);
			glm::quat rotz = glm::quat(0.0f, 0.0f, 1.0f, z);

			return rotx * roty * rotz;
		}
	};
};

