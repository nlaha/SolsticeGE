#include "EngineWrapper.h"

using namespace SolsticeGE;

VideoSettings EngineWrapper::videoSettings = {
    2560,
    1440,
    bgfx::RendererType::Direct3D12
};

bool EngineWrapper::enableStats = false;

AssetLibrary EngineWrapper::assetLib;
std::unordered_map<std::string, bgfx::UniformHandle> EngineWrapper::shaderUniforms;
std::unordered_map<std::string, bgfx::UniformHandle> EngineWrapper::shaderSamplers;

bgfx::FrameBufferHandle EngineWrapper::gbuffer;

const bgfx::Caps* EngineWrapper::renderCaps;

bgfx::ProgramHandle EngineWrapper::lightProgram;

float EngineWrapper::texelHalf = 0.0f;
float EngineWrapper::dt = 0.0f;

entt::entity EngineWrapper::activeCamera;

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

    // Load assets
    if (!assetLib.loadAssets("assets"))
    {
        spdlog::error("Could not load assets from directory!");
    }

    // Initialize game systems
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


    const bgfx::RendererType::Enum renderer = bgfx::getRendererType();
    texelHalf = bgfx::RendererType::Direct3D9 == renderer ? 0.5f : 0.0f;
    renderCaps = bgfx::getCaps();

    bgfx::setDebug(BGFX_DEBUG_STATS | BGFX_DEBUG_WIREFRAME);

    // test some ECS

    bgfx::ShaderHandle vshader = RenderUtil::loadShader("vs_mesh.bin");
    bgfx::ShaderHandle fshader = RenderUtil::loadShader("fs_mesh.bin");
    bgfx::ProgramHandle program = bgfx::createProgram(vshader, fshader, true);

    const auto entity = m_registry.create();
    m_registry.emplace<c_mesh>(entity, "assets\\DamagedHelmet.glb");
    m_registry.emplace<c_shader>(entity, vshader, fshader, program);

    m_registry.emplace<c_material>(entity, 
        "assets\\Default_albedo.dds",
        "assets\\Default_normal.dds",
        "assets\\Default_AO.dds",
        "assets\\Default_metalRoughness.dds",
        "assets\\Default_emissive.dds"
    );

    m_registry.emplace<c_transform>(entity,
        glm::vec3(0.0, 0.0, 0.0),
        glm::quat(glm::vec3(0, 45, 0)),
        glm::vec3(2.0, 2.0, 2.0)
    );

    const auto light = m_registry.create();
    m_registry.emplace<c_transform>(light,
        glm::vec3(3.0, 4.0, 3.0),
        glm::quat(glm::vec3(0, 0, 0)),
        glm::vec3(1.0, 1.0, 1.0)
        );
    m_registry.emplace<c_light>(light,
        1.0f,
        glm::vec3(8.0f, 0.8f, 0.0f),
        glm::vec3(1.0, 1.0, 1.0),
        2.0f,
        1.8f
    );

    // create camera
    const auto camera = m_registry.create();
    m_registry.emplace<c_camera>(camera, 
        90.0f, 
        0.001f, 
        1000.0f, 
        glm::vec2(
            videoSettings.windowWidth, 
            videoSettings.windowHeight),
        std::vector<RenderPass>({ 
            {kRenderPassGeometry, false},
            {kRenderPassLight, true},
            {kRenderPassCombine, true},
        })
    );

    m_registry.emplace<c_transform>(camera,
        glm::vec3(0.0, 0.0, 3.5),
        glm::angleAxis(glm::radians(-20.0f), glm::vec3(1.0f, 0.0f, 0.0f)),
        glm::vec3(1.0, 1.0, 1.0)
        );
    activeCamera = camera;

    // Set palette color for black
    bgfx::setPaletteColor(0, UINT32_C(0x00000000));

    // Set palette color for grey
    bgfx::setPaletteColor(1, UINT32_C(0x303030ff));

    // Set geometry pass view clear state.
    bgfx::setViewClear(kRenderPassGeometry
        , BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH
        , 1.0f
        , 0
        , 0
        , 0
        , 0
        , 0
        , 0
        , 0
    );

    // Set light pass view clear state.
    bgfx::setViewClear(kRenderPassLight
        , BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH
        , 1.0f
        , 0
        , 0
    );

    const uint64_t tsFlags = 0
        | BGFX_SAMPLER_MIN_POINT
        | BGFX_SAMPLER_MAG_POINT
        | BGFX_SAMPLER_MIP_POINT
        | BGFX_SAMPLER_U_CLAMP
        | BGFX_SAMPLER_V_CLAMP
        ;

    bgfx::Attachment gbufferAt[7];
    bgfx::TextureHandle m_gbufferTex[7];

    // geometry buffer tex
    // color
    m_gbufferTex[0] = bgfx::createTexture2D(
        uint16_t(videoSettings.windowWidth), 
        uint16_t(videoSettings.windowHeight), 
        false, 2, bgfx::TextureFormat::BGRA8, BGFX_TEXTURE_RT | tsFlags
    );
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
        false, 1, bgfx::TextureFormat::BGRA8, BGFX_TEXTURE_RT | tsFlags);
    gbufferAt[2].init(m_gbufferTex[2]);

    // AO
    m_gbufferTex[3] = bgfx::createTexture2D(
        uint16_t(videoSettings.windowWidth),
        uint16_t(videoSettings.windowHeight),
        false, 1, bgfx::TextureFormat::BGRA8, BGFX_TEXTURE_RT | tsFlags
    );
    gbufferAt[3].init(m_gbufferTex[3]);

    // Metalness / Roughness
    m_gbufferTex[4] = bgfx::createTexture2D(
        uint16_t(videoSettings.windowWidth),
        uint16_t(videoSettings.windowHeight),
        false, 1, bgfx::TextureFormat::BGRA8, BGFX_TEXTURE_RT | tsFlags
    );
    gbufferAt[4].init(m_gbufferTex[4]);

    // Emissive
    m_gbufferTex[5] = bgfx::createTexture2D(
        uint16_t(videoSettings.windowWidth),
        uint16_t(videoSettings.windowHeight),
        false, 1, bgfx::TextureFormat::BGRA8, BGFX_TEXTURE_RT | tsFlags
    );
    gbufferAt[5].init(m_gbufferTex[5]);

    // depth buffer tex
    bgfx::TextureFormat::Enum depthFormat =
        bgfx::isTextureValid(0, false, 1, bgfx::TextureFormat::D32F, BGFX_TEXTURE_RT | tsFlags)
        ? bgfx::TextureFormat::D32F : bgfx::TextureFormat::D24;

    m_gbufferTex[6] = bgfx::createTexture2D(
        uint16_t(videoSettings.windowWidth),
        uint16_t(videoSettings.windowHeight),
        false, 1, depthFormat, BGFX_TEXTURE_RT | tsFlags);
    gbufferAt[6].init(m_gbufferTex[6]);

    // light buffer tex
    bgfx::TextureHandle m_lightBufferTex;
    m_lightBufferTex = bgfx::createTexture2D(
        uint16_t(videoSettings.windowWidth),
        uint16_t(videoSettings.windowHeight), 
        false, 1, bgfx::TextureFormat::BGRA8, BGFX_TEXTURE_RT | tsFlags);

    // set up framebuffers
    gbuffer = bgfx::createFrameBuffer(7, gbufferAt, true);

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
    EngineWrapper::shaderSamplers.emplace("depth",
        bgfx::createUniform("s_depth", bgfx::UniformType::Sampler));

    EngineWrapper::shaderSamplers.emplace("normal",
        bgfx::createUniform("s_normal", bgfx::UniformType::Sampler));
    EngineWrapper::shaderSamplers.emplace("position",
        bgfx::createUniform("s_position", bgfx::UniformType::Sampler));
    EngineWrapper::shaderSamplers.emplace("ao",
        bgfx::createUniform("s_ao", bgfx::UniformType::Sampler));
    EngineWrapper::shaderSamplers.emplace("metalRoughness",
        bgfx::createUniform("s_metalRoughness", bgfx::UniformType::Sampler));
    EngineWrapper::shaderSamplers.emplace("emissive",
        bgfx::createUniform("s_emissive", bgfx::UniformType::Sampler));

    EngineWrapper::shaderSamplers.emplace("light",
        bgfx::createUniform("s_light", bgfx::UniformType::Sampler));

    // create other uniforms
    EngineWrapper::shaderUniforms.emplace(
        "viewPos", bgfx::createUniform("u_viewPos", bgfx::UniformType::Vec4));
    EngineWrapper::shaderUniforms.emplace(
        "normalMatrix", bgfx::createUniform("u_normalMatrix", bgfx::UniformType::Mat3));

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
    while (!glfwWindowShouldClose(mp_window)) {

        EngineWrapper::dt = bgfx::getStats()->cpuTimeFrame;

        glfwGetWindowSize(mp_window, 
            &videoSettings.windowWidth, 
            &videoSettings.windowHeight);

        bgfx::touch(0);

        // TODO: move to render thread (see bgfx docs)
        for (std::unique_ptr<System>& sys : m_renderSystems)
        {
            sys->update(m_registry);
        }

        // combined pass
        bgfx::setTexture(0, EngineWrapper::shaderSamplers.at("light"), m_lightBufferTex);
        bgfx::setState(0
            | BGFX_STATE_WRITE_RGB
            | BGFX_STATE_WRITE_A
        );

        screenSpaceQuad(
            videoSettings.windowWidth,
            videoSettings.windowHeight,
            texelHalf, renderCaps->originBottomLeft);
        bgfx::submit(kRenderPassCombine, m_combineProgram);

        bgfx::frame();

        glfwPollEvents();
    }

    return true;
}

void EngineWrapper::screenSpaceQuad(float _textureWidth, float _textureHeight, float _texelHalf, bool _originBottomLeft, float _width, float _height)
{
    if (3 == bgfx::getAvailTransientVertexBuffer(3, PassVertex::ms_layout))
    {
        bgfx::TransientVertexBuffer vb;
        bgfx::allocTransientVertexBuffer(&vb, 3, PassVertex::ms_layout);
        PassVertex* vertex = (PassVertex*)vb.data;

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
