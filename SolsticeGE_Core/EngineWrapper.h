#pragma once
#include <bgfx/bgfx.h>
#include <GLFW/glfw3.h>
#include <spdlog/spdlog.h>
#include <entt/entt.hpp>
#include <string>

#if BX_PLATFORM_LINUX
#define GLFW_EXPOSE_NATIVE_X11
#elif BX_PLATFORM_WINDOWS
#define GLFW_EXPOSE_NATIVE_WIN32
#elif BX_PLATFORM_OSX
#define GLFW_EXPOSE_NATIVE_COCOA
#endif
#include <GLFW/glfw3native.h>

#include "System.h"
#include "RenderCommon.h"
#include "AssetLibrary.h"

// systems
#include "MeshRenderSystem.h"
#include "CameraRenderSystem.h"
#include "BufferLoaderSystem.h"
#include "LightRenderSystem.h"

namespace SolsticeGE {

	constexpr bgfx::ViewId kRenderPassGeometry = 0;
	constexpr bgfx::ViewId kRenderPassClearUav = 1;
	constexpr bgfx::ViewId kRenderPassLight = 2;
	constexpr bgfx::ViewId kRenderPassCombine = 3;

	struct VideoSettings
	{
		int windowWidth;
		int windowHeight;
		bgfx::RendererType::Enum graphicsApi;
	};

	class EngineWrapper
	{
	public:

		EngineWrapper();
		~EngineWrapper();

		// delete copy constructor, it doesn't
		// make sense to copy a wrapper
		EngineWrapper(const EngineWrapper& other) = delete;
		void operator=(EngineWrapper const&) = delete;

		bool init();
		bool run();

		static bool enableStats;
		static VideoSettings videoSettings;
		static AssetLibrary assetLib;
		static std::unordered_map<std::string, bgfx::UniformHandle> shaderUniforms;
		static std::unordered_map<std::string, bgfx::UniformHandle> shaderSamplers;

		static float dt;

		static void screenSpaceQuad(
			float _textureWidth, float _textureHeight, 
			float _texelHalf, bool _originBottomLeft, 
			float _width = 1.0f, float _height = 1.0f);

		static bgfx::FrameBufferHandle gbuffer;
		static float texelHalf;
		static const bgfx::Caps* renderCaps;
		static bgfx::ProgramHandle lightProgram;

		static entt::entity activeCamera;

	private:
		GLFWwindow* mp_window;

		// ECS
		entt::registry m_registry;
		std::vector<std::unique_ptr<System>> m_gameSystems;
		std::vector<std::unique_ptr<System>> m_renderSystems;
		std::vector<std::unique_ptr<System>> m_backgroundSystems;

	};
}

