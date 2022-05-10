#pragma once
#include <bgfx/bgfx.h>
#include <GLFW/glfw3.h>
#include <spdlog/spdlog.h>
#include <entt/entt.hpp>

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

// systems
#include "MeshRenderSystem.h"
#include "CameraRenderSystem.h"
#include "ModelLoaderSystem.h"

namespace SolsticeGE {

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

		bool init();
		bool run();

		static bool enableStats;

	private:
		GLFWwindow* mp_window;
		static VideoSettings videoSettings;

		// ECS
		entt::registry m_registry;
		std::vector<std::unique_ptr<System>> m_gameSystems;
		std::vector<std::unique_ptr<System>> m_renderSystems;
		std::vector<std::unique_ptr<System>> m_backgroundSystems;

	};
}

