#include "EngineWrapper.h"

using namespace SolsticeGE;

VideoSettings EngineWrapper::videoSettings = {
    2560,
    1440,
    bgfx::RendererType::Direct3D12};

bool EngineWrapper::enableStats = false;

AssetLibrary EngineWrapper::assetLib;
std::unordered_map<std::string, bgfx::UniformHandle> EngineWrapper::shaderUniforms;
std::unordered_map<std::string, bgfx::UniformHandle> EngineWrapper::shaderSamplers;

bgfx::FrameBufferHandle EngineWrapper::gbuffer;

const bgfx::Caps *EngineWrapper::renderCaps;

bgfx::ProgramHandle EngineWrapper::lightProgram;

float EngineWrapper::texelHalf = 0.0f;
float EngineWrapper::dt = 0.0f;

// mesh shading
bgfx::ShaderHandle EngineWrapper::vs_mesh;
bgfx::ShaderHandle EngineWrapper::fs_mesh;
bgfx::ProgramHandle EngineWrapper::prog_mesh;

entt::entity EngineWrapper::activeCamera;

int EngineWrapper::gbufferDebugMode = -1;

MouseData EngineWrapper::userInput = {
    0.0, 0.0,
    EngineWrapper::videoSettings.windowWidth / 2.0f,
    EngineWrapper::videoSettings.windowHeight / 2.0f,
    -90.0f, 0.0f,
    true,
    glm::vec3(0.0, 0.0, -1.0)
};

/// <summary>
/// GLFW Error callback, called when there's a GLFW error
/// </summary>
/// <param name="error"></param>
/// <param name="description"></param>
static void glfwErrorCallback(int error, const char *description)
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
static void glfwKeyCallback(GLFWwindow *window, int key, int scancode, int action, int mods)
{
    if (action == GLFW_PRESS)
        InputManager::recieveBtnDown(key);

    if (action == GLFW_RELEASE)
        InputManager::recieveBtnUp(key);

    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GLFW_TRUE);

    if (key == GLFW_KEY_F1 && action == GLFW_PRESS)
    {
        EngineWrapper::enableStats = !EngineWrapper::enableStats;
        spdlog::info("Toggled debug stats: {}", EngineWrapper::enableStats);
    }

    if (key == GLFW_KEY_F2 && action == GLFW_PRESS)
        EngineWrapper::gbufferDebugMode = -1;
    if (key == GLFW_KEY_F3 && action == GLFW_PRESS)
        EngineWrapper::gbufferDebugMode = 0;
    if (key == GLFW_KEY_F4 && action == GLFW_PRESS)
        EngineWrapper::gbufferDebugMode = 1;
    if (key == GLFW_KEY_F5 && action == GLFW_PRESS)
        EngineWrapper::gbufferDebugMode = 2;
    if (key == GLFW_KEY_F6 && action == GLFW_PRESS)
        EngineWrapper::gbufferDebugMode = 3;
    if (key == GLFW_KEY_F7 && action == GLFW_PRESS)
        EngineWrapper::gbufferDebugMode = 4;
    if (key == GLFW_KEY_F8 && action == GLFW_PRESS)
        EngineWrapper::gbufferDebugMode = 5;

    if (EngineWrapper::enableStats)
    {
        bgfx::setDebug(BGFX_DEBUG_STATS);
    }
    else
    {
        bgfx::setDebug(BGFX_DEBUG_NONE);
    }
}

static void glfwMouseCallback(GLFWwindow* window, double xpos, double ypos)
{
    if (EngineWrapper::userInput.firstMouse)
    {
        EngineWrapper::userInput.mouseLastX = xpos;
        EngineWrapper::userInput.mouseLastY = ypos;
        EngineWrapper::userInput.firstMouse = false;
    }

    float xoffset = xpos - EngineWrapper::userInput.mouseLastX;
    float yoffset = EngineWrapper::userInput.mouseLastY - ypos;
    EngineWrapper::userInput.mouseLastX = xpos;
    EngineWrapper::userInput.mouseLastY = ypos;

    float sensitivity = 0.1f;
    xoffset *= sensitivity;
    yoffset *= sensitivity;

    EngineWrapper::userInput.yaw += xoffset;
    EngineWrapper::userInput.pitch += yoffset;

    if (EngineWrapper::userInput.pitch > 89.0f)
        EngineWrapper::userInput.pitch = 89.0f;
    if (EngineWrapper::userInput.pitch < -89.0f)
        EngineWrapper::userInput.pitch = -89.0f;

    glm::vec3 direction;
    direction.x = cos(glm::radians(EngineWrapper::userInput.yaw)) 
        * cos(glm::radians(EngineWrapper::userInput.pitch));
    direction.y = sin(glm::radians(EngineWrapper::userInput.pitch));
    direction.z = sin(glm::radians(EngineWrapper::userInput.yaw)) 
        * cos(glm::radians(EngineWrapper::userInput.pitch));
    EngineWrapper::userInput.cameraFront = glm::normalize(direction);
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

    // Load assets
    if (!assetLib.loadAssets("assets"))
    {
        spdlog::error("Could not load assets from directory!");
    }

    // Initialize game systems
    m_gameSystems.push_back(std::move(std::make_unique<SceneSpawnerSystem>()));
    m_gameSystems.push_back(std::move(std::make_unique<PlayerControllerSystem>()));
    m_gameSystems.push_back(std::move(std::make_unique<SceneHierarchySystem>()));

    // Initialize render systems
    m_renderSystems.push_back(std::move(std::make_unique<CameraRenderSystem>()));
    m_renderSystems.push_back(std::move(std::make_unique<BufferLoaderSystem>()));
    m_renderSystems.push_back(std::move(std::make_unique<MeshRenderSystem>()));
    m_renderSystems.push_back(std::move(std::make_unique<LightRenderSystem>()));

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
    if (!glfwInit())
    {
        spdlog::error("Could not initialize GLFW!");
        return false;
    }

    mp_window = glfwCreateWindow(
        videoSettings.windowWidth,
        videoSettings.windowHeight,
        "Solstice Engine", NULL, NULL);

    if (!mp_window)
    {
        spdlog::error("Could not initialize window!");
        return false;
    }
    else
    {
        spdlog::info("Initialized window with resolution: {}x{}",
                     videoSettings.windowWidth, videoSettings.windowHeight);
    }

    // set glfw key callback
    glfwSetKeyCallback(mp_window, glfwKeyCallback);

    // hide cursor
    glfwSetInputMode(mp_window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // set glfw mouse callback
    glfwSetCursorPosCallback(mp_window, glfwMouseCallback);

    // initialize bgfx
    bgfx::Init bgfxInit;
#if BX_PLATFORM_LINUX || BX_PLATFORM_BSD
    bgfxInit.platformData.ndt = glfwGetX11Display();
    bgfxInit.platformData.nwh = (void *)(uintptr_t)glfwGetX11Window(window);
#elif BX_PLATFORM_OSX
    bgfxInit.platformData.nwh = glfwGetCocoaWindow(window);
#elif BX_PLATFORM_WINDOWS
    bgfxInit.platformData.nwh = glfwGetWin32Window(mp_window);
#endif
    bgfxInit.type = videoSettings.graphicsApi;
    bgfxInit.resolution.width = videoSettings.windowWidth;
    bgfxInit.resolution.height = videoSettings.windowHeight;
    bgfxInit.resolution.reset = BGFX_RESET_MSAA_X4;

    if (bgfx::init(bgfxInit))
    {
        spdlog::info("Rendering engine initialized successfully, using render type {}", bgfx::getRendererType());
    }

    const bgfx::RendererType::Enum renderer = bgfx::getRendererType();
    texelHalf = bgfx::RendererType::Direct3D9 == renderer ? 0.5f : 0.0f;
    renderCaps = bgfx::getCaps();

    bgfx::setDebug(BGFX_DEBUG_STATS | BGFX_DEBUG_WIREFRAME);

    // create mesh shader program
    EngineWrapper::vs_mesh = RenderUtil::loadShader("vs_mesh.bin");
    EngineWrapper::fs_mesh = RenderUtil::loadShader("fs_mesh.bin");
    EngineWrapper::prog_mesh = bgfx::createProgram(
        EngineWrapper::vs_mesh, EngineWrapper::fs_mesh, true);

    // test some ECS

    const auto player = m_registry.create();
    m_registry.emplace<c_transform>(player,
        glm::vec3(0.0f, 0.0f, 0.0f),
        glm::vec3(0.0f, 0.0f, 0.0f),
        glm::vec3(1.0f, 1.0f, 1.0f));
    m_registry.emplace<c_player>(player);
    m_registry.emplace<c_camera>(player,
        70.0f,
        0.001f,
        10000.0f,
        glm::vec2(
            videoSettings.windowWidth,
            videoSettings.windowHeight),
        std::vector<RenderPass>({
            {kRenderPassGeometry, false},
            {kRenderPassLight, true},
            {kRenderPassCombine, true},
            }));

    EngineWrapper::activeCamera = player;

    const auto entity = m_registry.create();
    m_registry.emplace<c_scene>(entity, "assets\\imc_spider_tank\\scene.gltf", false);
    m_registry.emplace<c_transform>(entity,
        glm::vec3(0.0f, 0.0f, -0.7f),
        glm::vec3(glm::radians(90.0f), glm::radians(-90.0f), glm::radians(180.0f)),
        glm::vec3(1.0f, 1.0f, 1.0f));

    const auto light = m_registry.create();
    m_registry.emplace<c_transform>(light,
        glm::vec3(1.0f, 1.0f, 1.0f),
        glm::vec3(0.0f, 0.0f, 0.0f),
        glm::vec3(1.0f, 1.0f, 1.0f));
    m_registry.emplace<c_light>(light,
        kLightDirectional,
        glm::vec3(500.0f, 0.0f, 0.0f),
        glm::vec3(2.0f, 2.0f, 2.0f));

    // Set palette color for black
    bgfx::setPaletteColor(0, UINT32_C(0x00000000));

    // Set palette color for grey
    bgfx::setPaletteColor(1, UINT32_C(0x303030ff));

    // Set geometry pass view clear state.
    bgfx::setViewClear(kRenderPassGeometry, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH, 1.0f, 0, 0, 0, 0, 0, 0, 0);

    // Set light pass view clear state.
    bgfx::setViewClear(kRenderPassLight, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH, 1.0f, 0, 0);

    const uint64_t tsFlags = 0 | BGFX_SAMPLER_MIN_POINT | BGFX_SAMPLER_MAG_POINT | BGFX_SAMPLER_MIP_POINT | BGFX_SAMPLER_U_CLAMP | BGFX_SAMPLER_V_CLAMP;

    bgfx::Attachment gbufferAt[6];
    bgfx::TextureHandle m_gbufferTex[6];

    // geometry buffer tex
    // color
    m_gbufferTex[0] = bgfx::createTexture2D(
        uint16_t(videoSettings.windowWidth),
        uint16_t(videoSettings.windowHeight),
        false, 2, bgfx::TextureFormat::BGRA8, BGFX_TEXTURE_RT | tsFlags);
    gbufferAt[0].init(m_gbufferTex[0]);

    // normal
    // packed into two 16 bit channels (see shader for more info)
    m_gbufferTex[1] = bgfx::createTexture2D(
        uint16_t(videoSettings.windowWidth),
        uint16_t(videoSettings.windowHeight),
        false, 1, bgfx::TextureFormat::RG16F, BGFX_TEXTURE_RT | tsFlags);
    gbufferAt[1].init(m_gbufferTex[1]);

    // position
    m_gbufferTex[2] = bgfx::createTexture2D(
        uint16_t(videoSettings.windowWidth),
        uint16_t(videoSettings.windowHeight),
        false, 1, bgfx::TextureFormat::RGBA16F, BGFX_TEXTURE_RT | tsFlags);
    gbufferAt[2].init(m_gbufferTex[2]);

    // AO Metal Roughness
    m_gbufferTex[3] = bgfx::createTexture2D(
        uint16_t(videoSettings.windowWidth),
        uint16_t(videoSettings.windowHeight),
        false, 1, bgfx::TextureFormat::BGRA8, BGFX_TEXTURE_RT | tsFlags);
    gbufferAt[3].init(m_gbufferTex[3]);

    // Emissive
    m_gbufferTex[4] = bgfx::createTexture2D(
        uint16_t(videoSettings.windowWidth),
        uint16_t(videoSettings.windowHeight),
        false, 1, bgfx::TextureFormat::BGRA8, BGFX_TEXTURE_RT | tsFlags);
    gbufferAt[4].init(m_gbufferTex[4]);

    // depth buffer tex
    bgfx::TextureFormat::Enum depthFormat =
        bgfx::isTextureValid(0, false, 1, bgfx::TextureFormat::D32F, BGFX_TEXTURE_RT | tsFlags)
            ? bgfx::TextureFormat::D32F
            : bgfx::TextureFormat::D24;

    m_gbufferTex[5] = bgfx::createTexture2D(
        uint16_t(videoSettings.windowWidth),
        uint16_t(videoSettings.windowHeight),
        false, 1, depthFormat, BGFX_TEXTURE_RT | tsFlags);
    gbufferAt[5].init(m_gbufferTex[5]);

    // light buffer tex
    bgfx::TextureHandle m_lightBufferTex;
    m_lightBufferTex = bgfx::createTexture2D(
        uint16_t(videoSettings.windowWidth),
        uint16_t(videoSettings.windowHeight),
        false, 1, bgfx::TextureFormat::BGRA8, BGFX_TEXTURE_RT | tsFlags);

    // set up framebuffers
    // gbuffer
    gbuffer = bgfx::createFrameBuffer(6, gbufferAt, true);

    // light buffer
    bgfx::FrameBufferHandle m_lightBuffer;
    m_lightBuffer = bgfx::createFrameBuffer(1, &m_lightBufferTex, true);

    // set view framebuffers
    bgfx::setViewFrameBuffer(kRenderPassGeometry, gbuffer);
    bgfx::setViewFrameBuffer(kRenderPassLight, m_lightBuffer);

    // setup render pass shader programs
    bgfx::ShaderHandle combined_vshader = RenderUtil::loadShader("vs_combined.bin");
    bgfx::ShaderHandle combined_fshader = RenderUtil::loadShader("fs_combined.bin");
    bgfx::ProgramHandle m_combineProgram = bgfx::createProgram(combined_vshader, combined_fshader, true);

    bgfx::ShaderHandle lighting_vshader = RenderUtil::loadShader("vs_lighting.bin");
    bgfx::ShaderHandle lighting_fshader = RenderUtil::loadShader("fs_lighting.bin");
    lightProgram = bgfx::createProgram(lighting_vshader, lighting_fshader, true);

    // init vertex for drawing passes to screen
    PassVertex::init();

    // Init vertex for drawing other things
    BasicVertex::init();

    // setup render pass samplers
    EngineWrapper::shaderSamplers.emplace("albedo",
                                          bgfx::createUniform("s_albedo", bgfx::UniformType::Sampler));
    EngineWrapper::shaderSamplers.emplace("normal",
                                          bgfx::createUniform("s_normal", bgfx::UniformType::Sampler));
    EngineWrapper::shaderSamplers.emplace("position",
                                          bgfx::createUniform("s_position", bgfx::UniformType::Sampler));
    EngineWrapper::shaderSamplers.emplace("ao_metal_rough",
                                          bgfx::createUniform("s_ao_metal_rough", bgfx::UniformType::Sampler));
    EngineWrapper::shaderSamplers.emplace("emissive",
                                          bgfx::createUniform("s_emissive", bgfx::UniformType::Sampler));
    EngineWrapper::shaderSamplers.emplace("depth",
                                          bgfx::createUniform("s_depth", bgfx::UniformType::Sampler));

    EngineWrapper::shaderSamplers.emplace("light",
                                          bgfx::createUniform("s_light", bgfx::UniformType::Sampler));

    // create other uniforms
    EngineWrapper::shaderUniforms.emplace(
        "viewPos", bgfx::createUniform("u_viewPos", bgfx::UniformType::Vec4));
    EngineWrapper::shaderUniforms.emplace(
        "normalMatrix", bgfx::createUniform("u_normalMatrix", bgfx::UniformType::Mat3));
    EngineWrapper::shaderUniforms.emplace(
        "isPacked", bgfx::createUniform("u_isPacked", bgfx::UniformType::Vec4));

    // lighting uniforms
    EngineWrapper::shaderUniforms.emplace(
        "lightPosition", bgfx::createUniform("u_lightPosition", bgfx::UniformType::Vec4, 1));
    EngineWrapper::shaderUniforms.emplace(
        "lightColor", bgfx::createUniform("u_lightColor", bgfx::UniformType::Vec4, 1));
    EngineWrapper::shaderUniforms.emplace(
        "lightTypeParams", bgfx::createUniform("u_lightTypeParams", bgfx::UniformType::Vec4, 1));

    // Set view 0 default viewport.
    bgfx::setViewRect(0, 0, 0,
                      uint16_t(videoSettings.windowWidth),
                      uint16_t(videoSettings.windowHeight));

    // main loop
    while (!glfwWindowShouldClose(mp_window))
    {
        auto start = std::chrono::high_resolution_clock::now();

        glfwGetWindowSize(mp_window,
                          &videoSettings.windowWidth,
                          &videoSettings.windowHeight);

        bgfx::touch(0);

        // call update on game systems
        for (std::unique_ptr<System>& sys : m_gameSystems)
        {
            sys->update(m_registry);
        }

        // TODO: move to render thread (see bgfx docs)
        for (std::unique_ptr<System> &sys : m_renderSystems)
        {
            sys->update(m_registry);
        }

        // equirectangular maps to cube textures
        // TODO: figure this out, see link:
        // https://learnopengl.com/PBR/IBL/Diffuse-irradiance
        //std::weak_ptr<AssetLibrary::Texture> env_cubemap;
        //if (EngineWrapper::assetLib.getCubemap(0, env_cubemap)) {
        //    bgfx::setTexture(0,
        //        env_cubemap.lock()->sampler,
        //        env_cubemap.lock()->texHandle);
        //}

        //bgfx::setState(0 | BGFX_STATE_WRITE_RGB | BGFX_STATE_WRITE_A);

        //screenSpaceQuad(
        //    videoSettings.windowWidth,
        //    videoSettings.windowHeight,
        //    texelHalf, renderCaps->originBottomLeft);
        //bgfx::submit(kRenderPassEnvironment, m_envProgram);

        // combined pass
        if (EngineWrapper::gbufferDebugMode == -1) {
            bgfx::setTexture(0,
                EngineWrapper::shaderSamplers.at("light"),
                m_lightBufferTex);
        }
        else {
            bgfx::setTexture(0,
                EngineWrapper::shaderSamplers.at("light"),
                bgfx::getTexture(gbuffer, EngineWrapper::gbufferDebugMode));
        }

        bgfx::setState(0 | BGFX_STATE_WRITE_RGB | BGFX_STATE_WRITE_A);

        screenSpaceQuad(
            videoSettings.windowWidth,
            videoSettings.windowHeight,
            texelHalf, renderCaps->originBottomLeft);
        bgfx::submit(kRenderPassCombine, m_combineProgram);

        bgfx::frame();

        glfwPollEvents();

        auto end = std::chrono::high_resolution_clock::now();
        float dt = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count() / 1000000.0f;

        EngineWrapper::dt = dt;
    }

    return true;
}

/// <summary>
/// Submits the vertex buffer for a triangle that
/// covers the whole screen, used for deferred
/// render passes
/// </summary>
/// <param name="_textureWidth"></param>
/// <param name="_textureHeight"></param>
/// <param name="_texelHalf"></param>
/// <param name="_originBottomLeft"></param>
/// <param name="_width"></param>
/// <param name="_height"></param>
void EngineWrapper::screenSpaceQuad(float _textureWidth, float _textureHeight, float _texelHalf, bool _originBottomLeft, float _width, float _height)
{
    if (3 == bgfx::getAvailTransientVertexBuffer(3, PassVertex::ms_layout))
    {
        bgfx::TransientVertexBuffer vb;
        bgfx::allocTransientVertexBuffer(&vb, 3, PassVertex::ms_layout);
        PassVertex *vertex = (PassVertex *)vb.data;

        const float minx = -_width;
        const float maxx = _width;
        const float miny = 0.0f;
        const float maxy = _height * 2.0f;

        const float texelHalfW = _texelHalf / _textureWidth;
        const float texelHalfH = _texelHalf / _textureHeight;
        const float minu = -1.0f + texelHalfW;
        const float maxu = 1.0f + texelHalfH;

        const float zz = 0.0f;

        float minv = texelHalfH;
        float maxv = 2.0f + texelHalfH;

        if (_originBottomLeft)
        {
            float temp = minv;
            minv = maxv;
            maxv = temp;

            minv -= 1.0f;
            maxv -= 1.0f;
        }

        vertex[0].m_x = minx;
        vertex[0].m_y = miny;
        vertex[0].m_z = zz;
        vertex[0].m_u = minu;
        vertex[0].m_v = minv;

        vertex[1].m_x = maxx;
        vertex[1].m_y = miny;
        vertex[1].m_z = zz;
        vertex[1].m_u = maxu;
        vertex[1].m_v = minv;

        vertex[2].m_x = maxx;
        vertex[2].m_y = maxy;
        vertex[2].m_z = zz;
        vertex[2].m_u = maxu;
        vertex[2].m_v = maxv;

        bgfx::setVertexBuffer(0, &vb);
    }
}
