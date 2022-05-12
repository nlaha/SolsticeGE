#pragma once

#include <bgfx/bgfx.h>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>
#include <array>
#include <glm/gtc/quaternion.hpp>

#include "AssetLibrary.h"
#include "RenderCommon.h"

namespace SolsticeGE {

	/*
	Contains components used for rendering.
	Keep in mind, components in ECS will
	never have functionality, this is data only!

	HOWEVER...

	there is an exception in our use case, since
	all of our data is extremely un-coupled from
	functionality, this makes it super easy to add
	network serialization, so we'll do that here.

	*/


	/// <summary>
	/// Holds transform data
	/// model matrix is computed
	/// in the render thread
	/// </summary>
	struct c_transform {

		glm::vec3 pos;
		glm::quat rot;
		glm::vec3 scale;
	};

	/// <summary>
	/// Holds mesh data
	/// Note: this doesn't
	/// contain shader info
	/// </summary>
	struct c_mesh {
		std::string assetId;
	};

	/// <summary>
	/// Holds shader program
	/// data, contains vertex
	/// and fragment shader handles
	/// </summary>
	struct c_shader {
		bgfx::ShaderHandle vshader;
		bgfx::ShaderHandle fshader;
		bgfx::ProgramHandle program;
	};

	/// <summary>
	/// Holds material data,
	/// textures, parameters, etc.
	/// </summary>
	struct c_material {
		std::string diffuse_tex;
		std::string normal_tex;
		std::string ao_tex;
		std::string metalRoughness_tex;
		std::string emissive_tex;
	};

	/// <summary>
	/// Holds data for lights
	/// </summary>
	struct c_light {

		// 0 = directional, 1 = point
		float type;

		/*
			This is different for each type
			Directional: [Unused, Unused, Unused]
			Point: [OuterRadius, InnerRadius, Unused]
		*/
		glm::vec3 params;
		glm::vec3 color;
		float outer;
		float inner;
	};

	/// <summary>
	/// Holds camera data
	/// </summary>
	struct c_camera {
		float fov;
		float clipNear;
		float clipFar;
		glm::vec2 size;
		std::vector<RenderPass> passes;

		glm::mat4 viewMatrix;
		glm::mat4 projMatrix;
	};

}