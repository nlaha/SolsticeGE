#include "EngineWrapper.h"

using namespace SolsticeGE;

VideoSettings EngineWrapper::videoSettings = {
    2560,
    1440,
    bgfx::RendererType::Direct3D12
};

bool EngineWrapper::enableStats = false;

/// <summary>
/// GLFW Error callback, called when there's a GLFW error
/// </summary>
/// <param name="error"></param>
/// <param name="description"></param>
static void glfwErrorCallback(int error, const char* description)
{
    spdlog::error("GLFW Error (Code {}): {}\n", error, description);
}

/// <summary>
/// GLFW Key callback, called when a user presses a key while
/// the window is focused
/// </summary>
/// <param name="window"></param>
/// <param name="key"></param>
/// <param name="scancode"></param>
/// <param name="action"></param>
/// <param name="mods"></param>
static void glfwKeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GLFW_TRUE);

    if (key == GLFW_KEY_F1 && action == GLFW_PRESS) {
        EngineWrapper::enableStats = !EngineWrapper::enableStats;
        spdlog::info("Toggled debug stats: {}", EngineWrapper::enableStats);
    }

    if (EngineWrapper::enableStats) {
        bgfx::setDebug(BGFX_DEBUG_STATS);
    }
    else {
        bgfx::setDebug(BGFX_DEBUG_NONE);
    }
}

/// <summary>
/// Default constructor
/// </summary>
EngineWrapper::EngineWrapper()
    : mp_window(nullptr)
{

}

/// <summary>
/// Destructor
/// </summary>
EngineWrapper::~EngineWrapper()
{
    spdlog::info("Thanks for using Solstice Engine! Cleaning things up...");

    // clean up application
    glfwDestroyWindow(mp_window);
    glfwTerminate();
}

/// <summary>
/// Initializes data 
/// before the window is created
/// </summary>
bool EngineWrapper::init()
{
    // TODO: load video settings from file

    // Initialize game systems
    m_renderSystems.push_back(std::move(std::make_unique<CameraRenderSystem>()));
    m_renderSystems.push_back(std::move(std::make_unique<ModelLoaderSystem>()));
    m_renderSystems.push_back(std::move(std::make_unique<MeshRenderSystem>()));

    return true;
}

/// <summary>
/// Starts the game engine
/// </summary>
bool EngineWrapper::run()
{
    // init glfw error callback
    glfwSetErrorCallback(glfwErrorCallback);

    // initialize glfw
    if (!glfwInit()) {
        spdlog::error("Could not initialize GLFW!");
        return false;
    }

    mp_window = glfwCreateWindow(
        videoSettings.windowWidth, 
        videoSettings.windowHeight, 
        "Solstice Engine", NULL, NULL);

    if (!mp_window) {
        spdlog::error("Could not initialize window!");
        return false;
    }
    else {
        spdlog::info("Initialized window with resolution: {}x{}", 
            videoSettings.windowWidth, videoSettings.windowHeight);
    }

    // set glfw key callback
    glfwSetKeyCallback(mp_window, glfwKeyCallback);

    // initialize bgfx
    bgfx::Init bgfxInit;
#if BX_PLATFORM_LINUX || BX_PLATFORM_BSD
    bgfxInit.platformData.ndt = glfwGetX11Display();
    bgfxInit.platformData.nwh = (void*)(uintptr_t)glfwGetX11Window(window);
#elif BX_PLATFORM_OSX
    bgfxInit.platformData.nwh = glfwGetCocoaWindow(window);
#elif BX_PLATFORM_WINDOWS
    bgfxInit.platformData.nwh = glfwGetWin32Window(mp_window);
#endif
    bgfxInit.type = videoSettings.graphicsApi;
    bgfxInit.resolution.width = videoSettings.windowWidth;
    bgfxInit.resolution.height = videoSettings.windowHeight;
    bgfxInit.resolution.reset = BGFX_RESET_MSAA_X4;

    if (bgfx::init(bgfxInit)) {
        spdlog::info("Rendering engine initialized successfully, using render type {}", bgfx::getRendererType());
    }

    bgfx::setDebug(BGFX_DEBUG_STATS | BGFX_DEBUG_WIREFRAME);

    // test some ECS
    
    // Create vertex stream declaration.
    BasicVertex::init();

    bgfx::ShaderHandle vshader = RenderUtil::loadShader("vs_mesh.bin");
    bgfx::ShaderHandle fshader = RenderUtil::loadShader("fs_mesh.bin");
    bgfx::ProgramHandle program = bgfx::createProgram(vshader, fshader, true);

    const auto entity = m_registry.create();
    m_registry.emplace<c_model>(entity, "AntiqueCamera.glb");
    m_registry.emplace<c_mesh>(entity, false);
    m_registry.emplace<c_shader>(entity, vshader, fshader, program);
    m_registry.emplace<c_transform>(entity,
        glm::vec3(0.0, -4.0, 0.0),
        glm::angleAxis(glm::radians(-45.0f), glm::vec3(0.f, 1.f, 0.f)),
        glm::vec3(0.4, 0.4, 0.4)
    );

    // create camera
    const auto camera = m_registry.create();
    m_registry.emplace<c_camera>(camera, 
        90.0f, 
        0.001f, 
        1000.0f, 
        glm::vec2(
            videoSettings.windowWidth, 
            videoSettings.windowHeight)
    );

    m_registry.emplace<c_transform>(camera,
        glm::vec3(0.0, 0.0, 2.0),
        glm::angleAxis(glm::radians(-20.0f), glm::vec3(1.0f, 0.0f, 0.0f)),
        glm::vec3(1.0, 1.0, 1.0)
        );


    bgfx::setViewClear(0, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH, 0x222222FF, 1.0f, 0);

    // main loop
    while (!glfwWindowShouldClose(mp_window)) {

        glfwGetWindowSize(mp_window, 
            &videoSettings.windowWidth, 
            &videoSettings.windowHeight);

        bgfx::touch(0);

        // TODO: move to render thread (see bgfx docs)
        for (std::unique_ptr<System>& sys : m_renderSystems)
        {
            sys->update(m_registry);
        }

        bgfx::frame();

        glfwPollEvents();
    }

    return true;
}
