#pragma once
#include <bgfx/bgfx.h>
#include <string>
#include <spdlog/spdlog.h>
#include <fstream>

namespace SolsticeGE {

	struct RenderPass {
		bgfx::ViewId viewId;
		bool fullscreenOrtho;
	};

	/// <summary>
	/// Used for drawing screen
	/// space quads during render passes
	/// </summary>
	struct PassVertex
	{
		// pos
		float m_x;
		float m_y;
		float m_z;

		// uv0
		float m_u;
		float m_v;

		static void init()
		{
			ms_layout
				.begin()
				.add(bgfx::Attrib::Position, 3, bgfx::AttribType::Float)
				.add(bgfx::Attrib::TexCoord0, 2, bgfx::AttribType::Float)
				.end();
		};

		static bgfx::VertexLayout ms_layout;
	};

	struct BasicVertex
	{
		// pos
		float m_x;
		float m_y;
		float m_z;

		// normals
		float m_normx;
		float m_normy;
		float m_normz;

		// tangent
		float m_tanx;
		float m_tany;
		float m_tanz;

		// bitangent
		float m_btanx;
		float m_btany;
		float m_btanz;

		// uv0
		float m_u;
		float m_v;

		// vert color
		uint32_t m_abgr;

		static void init()
		{
			ms_layout
				.begin()
				.add(bgfx::Attrib::Position, 3, bgfx::AttribType::Float)
				.add(bgfx::Attrib::Normal, 3, bgfx::AttribType::Float)
				.add(bgfx::Attrib::Tangent, 3, bgfx::AttribType::Float)
				.add(bgfx::Attrib::Bitangent, 3, bgfx::AttribType::Float)
				.add(bgfx::Attrib::TexCoord0, 2, bgfx::AttribType::Float)
				.add(bgfx::Attrib::Color0, 4, bgfx::AttribType::Uint8, true)
				.end();
		};

		static bgfx::VertexLayout ms_layout;
	};

	class RenderUtil {
	
	public:
		static bgfx::ShaderHandle loadShader(const std::string& fname);
	
	};
}