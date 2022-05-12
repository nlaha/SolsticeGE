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
		std::uint16_t assetId;
	};

	/// <summary>
	/// Holds data about a model
	/// </summary>
	struct c_model {
		std::string filepath;
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
		std::uint16_t diffuse_tex;
		std::uint16_t normal_tex;
		std::uint16_t roughness_tex;

	};

	/// <summary>
	/// Point light
	/// </summary>
	struct c_light_point {
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