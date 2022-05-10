#pragma once
#include <bgfx/bgfx.h>
#include <string>
#include <spdlog/spdlog.h>
#include <fstream>

namespace SolsticeGE {

	struct BasicVertex
	{
		float m_x;
		float m_y;
		float m_z;
		float m_normx;
		float m_normy;
		float m_normz;
		float m_u;
		float m_v;
		uint32_t m_abgr;

		static void init()
		{
			ms_layout
				.begin()
				.add(bgfx::Attrib::Position, 3, bgfx::AttribType::Float)
				.add(bgfx::Attrib::Normal, 3, bgfx::AttribType::Float)
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