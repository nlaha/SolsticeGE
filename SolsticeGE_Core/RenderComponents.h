#pragma once

#include <bgfx/bgfx.h>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtc/quaternion.hpp>

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
		bool isValid;
		bgfx::VertexBufferHandle vbuf;
		bgfx::IndexBufferHandle ibuf;

		std::vector<BasicVertex> vdata;
		std::vector<uint16_t> idata;
	};

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

	struct c_material {
		std::vector<bgfx::TextureHandle> diffuse_tex;
		std::vector<bgfx::TextureHandle> normal_tex;
		std::vector<bgfx::TextureHandle> roughness_tex;

	};

	/// <summary>
	/// Holds camera data
	/// </summary>
	struct c_camera {
		float fov;
		float clipNear;
		float clipFar;
		glm::vec2 size;
	};

}