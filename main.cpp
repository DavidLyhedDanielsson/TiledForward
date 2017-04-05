#include <chrono>
#include <thread>
#include <set>
#include <glm/gtc/matrix_transform.hpp>
#include <X11/Xlib.h>
#include <random>

#include "os/window.h"
#include "timer.h"
#include "logger.h"
#include "os/input.h"
#include "perspectiveCamera.h"
#include "gl/glVertexBuffer.h"
#include "gl/glDrawBinds.h"
#include "content/texture.h"
#include "content/contentManager.h"
#include "spriteRenderer.h"
#include "console/guiManager.h"
#include "console/console.h"

#include "console/commandGetSet.h"
#include "content/OBJModel.h"
#include "console/colors.h"
#include "gl/glPixelShader.h"
#include "content/shaderContentParameters.h"
#include "console/commandCallMethod.h"
#include "primitiveDrawer.h"
#include "gl/glCPPShared.h"
#include "lightCullAdaptive.h"
#include "lightCullNormal.h"
#include "lightManager.h"
#include "lightCullClustered.h"

#include <glm/gtx/component_wise.hpp>
#include <IL/il.h>
#include <glm/gtx/norm.hpp>

class Main
{
public:
    Main();
    ~Main()
    { }

    int Run();
protected:
private:

    const static int DEFAULT_SCREEN_WIDTH = 1024;
    const static int DEFAULT_SCREEN_HEIGHT = 1024;

    int screenWidth = DEFAULT_SCREEN_WIDTH;
    int screenHeight = DEFAULT_SCREEN_HEIGHT;

    // TODO: Smart this up
    LightCullNormal* lightCullNormal;
    LightCullAdaptive* lightCullAdaptive;
    LightCullClustered* lightCullClustered;
    LightCull* currentLightCull;

    OSWindow window;

    std::set<KEY_CODE> keysDown;

    GUIManager guiManager;
    ContentManager contentManager;

    PrimitiveDrawer primitiveDrawer;

    SpriteRenderer spriteRenderer;
    Console console;

    CharacterSet* characterSet24;
    CharacterSet* characterSet8;

    OBJModel* worldModel;

    PerspectiveCamera camera;
    PerspectiveCamera snapshotCamera;
    PerspectiveCamera* currentCamera;

    // Debug
    bool wireframe;
    glm::ivec2 tileToDraw;

    GLuint frameBufferDepthOnly;
    //GLuint depthRenderBuffer;
    //GLuint backRenderBuffer;
    GLuint depthBufferTexture;
    GLuint backBufferTexture;

    float averageFrameTime;

    float lastMinFrameTime;
    float lastMaxFrameTime;

    float minFrameTime;
    float maxFrameTime;

    float cameraSpeed;

    GLuint queries[2];

    int msaaCount;

    bool recompileShaders;

    bool drawLightCount;
    bool dumpPreBackBuffer;
    bool dumpPostBackBuffer;

    LightManager lightManager;

    int InitContent();
    void InitConsole();
    void InitInput();
    bool InitFrameBuffers();
    void InitQuieries();
    bool InitShaders();

    void Update(Timer& deltaTimer);
    void Render(Timer& deltaTimer);
    bool ResizeFramebuffer(int width, int height, bool recreateBuffers);

    void DumpBackBuffer();
};

int main(int argc, char* argv[])
{
    Main main;
    return main.Run();
}

Main::Main()
        : contentManager("content")
          , wireframe(false)
          , tileToDraw(-1)
          , averageFrameTime(0.0f)
          , msaaCount(2)
          , depthBufferTexture(0)
          , backBufferTexture(0)
          , currentCamera(&camera)
          , recompileShaders(false)
          , cameraSpeed(0.01f)
          , drawLightCount(false)
          , dumpPreBackBuffer(false)
          , dumpPostBackBuffer(false)
          , lightCullNormal(nullptr)
          , lightCullAdaptive(nullptr)
          , lightCullClustered(nullptr)
{ }

float currentFrameTime = 0.0f;

int Main::Run()
{
    srand(1234);

    Logger::ClearLog();

    // TODO: Fix rename of index to blockIndex, e.g. "Used for index rounding" -> "Used for blockIndex rounding"

    if(window.Create((unsigned int)screenWidth, (unsigned int)screenHeight) == OSWindow::NONE)
    {
        if(!InitShaders())
            return 3;
        if(!InitFrameBuffers())
            return 2;

        InitInput();

        int errVal = InitContent();
        if(errVal != 0)
            return errVal;

        InitConsole();

        recompileShaders = true;

        // Use a reversed depth buffer
        glClipControl(GL_LOWER_LEFT, GL_ZERO_TO_ONE);
        glDepthRange(1.0, 0.0);
        camera.InitFovVertical({-10.0f, 5.5f, -2.0f}
                               , {0.0f, 0.0f, 0.0f}
                               , glm::half_pi<float>()
                               , screenWidth
                               , screenHeight
                               , 0.01f
                               , 100.0f);
        snapshotCamera = camera;

        primitiveDrawer.Init(contentManager);

        lightManager.AddConsoleCommands(console);
        InitQuieries();
        console.Autoexec();

        double frameTime = 0.0;
        unsigned long frameCount = 0;

        minFrameTime = std::numeric_limits<float>::max();
        maxFrameTime = std::numeric_limits<float>::min();

        const static int FRAME_CAP = 9999;

        Logger::ClearLog();

        Timer deltaTimer;
        deltaTimer.ResetDelta();
        while(window.PollEvents())
        {
            Timer frameCapTimer;
            frameCapTimer.Start();

            if(window.IsPaused())
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
                deltaTimer.ResetDelta();
                continue;
            }

            deltaTimer.UpdateDelta();

            currentFrameTime = deltaTimer.GetDeltaMillisecondsFraction();
            frameTime += currentFrameTime;
            ++frameCount;

            minFrameTime = std::min(minFrameTime, currentFrameTime);
            maxFrameTime = std::max(maxFrameTime, currentFrameTime);

            if(frameTime >= 1000)
            {
                averageFrameTime = 1.0f / frameCount * 1000.0f;

                frameCount = 0;
                frameTime = 0.0;

                lastMinFrameTime = minFrameTime;
                lastMaxFrameTime = maxFrameTime;

                minFrameTime = std::numeric_limits<float>::max();
                maxFrameTime = std::numeric_limits<float>::min();
            }

            Update(deltaTimer);
            Render(deltaTimer);

            frameCapTimer.Stop();
            auto time = frameCapTimer.GetTimeNanoseconds();
            if(time < 1.0 / FRAME_CAP * 1e9)
                std::this_thread::sleep_for(std::chrono::nanoseconds((int)(1.0 / FRAME_CAP * 1e9) - time));

            Input::Update();
        }
    }
    else
        Logger::LogLine(LOG_TYPE::FATAL, "Couldn't create window");

    Logger::SetCallOnLog(nullptr);

    return 0;
}

int Main::InitContent()
{
    // Creates "whiteTexture", so should be first
    if(!spriteRenderer.Init(contentManager, screenWidth, screenHeight))
    {
        Logger::LogLine(LOG_TYPE::FATAL, "Couldn't initialize sprite renderer");
        return 1;
    }

    characterSet24 = contentManager.Load<CharacterSet>("UbuntuMono-R24.ttf");
    characterSet8 = contentManager.Load<CharacterSet>("UbuntuMono-R8.ttf");

    OBJModelParameters parameters;
    parameters.shaderPath = currentLightCull->GetForwardShaderPath();
    worldModel = contentManager.Load<OBJModel>("models/sponza.obj", &parameters);
    if(worldModel == nullptr)
        return 3;

    currentLightCull->SetDrawBindData(worldModel->drawBinds);

    return 0;
}

void Main::InitConsole()
{
    console.Init(&contentManager
                 , Rect(0.0f, 0.0f, screenWidth, screenHeight / 2.0f)
                 , console.GenerateDoomStyle(&contentManager, characterSet24)
                 , console.GenerateDoomStyleBackgroundStyle(&contentManager)
                 , false
                 , false
                 , false
                 , false);
    console.AddCommand(new CommandGetSet<bool>("wireframe", &wireframe));
    console.AddCommand(new CommandGetSet<bool>("light_drawCount", &drawLightCount));
    console.AddCommand(new CommandGetSet<float>("cameraSpeed", &cameraSpeed));

    console.AddCommand(new CommandCallMethod("light_SetCullMode"
                                             , [&](const std::vector<Argument>& args)
            {
                if(args.size() != 1)
                    return Argument("Expected 1 argument");

                std::string arg = args.front().value;

                bool validArg = true;
                bool debug = false;

                if(arg == "normal")
                {
                    if(lightCullNormal == nullptr)
                    {
                        lightCullNormal = new LightCullNormal();

                        lightCullNormal->InitShaderConstants(screenWidth, screenHeight);
                        if(!lightCullNormal->Init(contentManager, console))
                        {
                            delete lightCullNormal;
                            return Argument("Couldn't initialize shader");
                        }
                    }

                    currentLightCull = lightCullNormal;
                }
                else if(arg == "adaptive")
                {
                    if(lightCullAdaptive == nullptr)
                    {
                        lightCullAdaptive = new LightCullAdaptive();

                        lightCullAdaptive->InitShaderConstants(screenWidth, screenHeight);
                        if(!lightCullAdaptive->Init(contentManager, console))
                        {
                            delete lightCullAdaptive;
                            return Argument("Couldn't initialize shader");
                        }
                    }

                    currentLightCull = lightCullAdaptive;
                }
                else if(arg == "normalDebug")
                {
                    if(lightCullNormal == nullptr)
                    {
                        lightCullNormal = new LightCullNormal();

                        lightCullNormal->InitShaderConstants(screenWidth, screenHeight);
                        if(!lightCullNormal->Init(contentManager, console))
                        {
                            delete lightCullNormal;
                            return Argument("Couldn't initialize shader");
                        }
                    }

                    currentLightCull = lightCullNormal;
                    debug = true;
                }
                else if(arg == "adaptiveDebug")
                {
                    if(lightCullAdaptive == nullptr)
                    {
                        lightCullAdaptive = new LightCullAdaptive();

                        lightCullAdaptive->InitShaderConstants(screenWidth, screenHeight);
                        if(!lightCullAdaptive->Init(contentManager, console))
                        {
                            delete lightCullAdaptive;
                            return Argument("Couldn't initialize shader");
                        }
                    }

                    currentLightCull = lightCullAdaptive;
                    debug = true;
                }
                else if(arg == "clustered")
                {
                    if(lightCullClustered == nullptr)
                    {
                        lightCullClustered = new LightCullClustered();

                        lightCullClustered->InitShaderConstants(screenWidth, screenHeight);
                        if(!lightCullClustered->Init(contentManager, console))
                        {
                            delete lightCullClustered;
                            return Argument("Couldn't initialize shader");
                        }
                    }

                    currentLightCull = lightCullClustered;
                    debug = true;
                }
                else if(arg == "clusteredDebug")
                {

                    if(lightCullClustered == nullptr)
                    {
                        lightCullClustered = new LightCullClustered();

                        lightCullClustered->InitShaderConstants(screenWidth, screenHeight);
                        if(!lightCullClustered->Init(contentManager, console))
                        {
                            delete lightCullClustered;
                            return Argument("Couldn't initialize shader");
                        }
                    }

                    currentLightCull = lightCullClustered;
                    debug = true;
                }
                else
                    validArg = false;

                if(validArg)
                {
                    std::string shaderPath;

                    if(debug)
                        shaderPath = currentLightCull->GetForwardShaderDebugPath();
                    else
                        shaderPath = currentLightCull->GetForwardShaderPath();

                    if(!worldModel->drawBinds.ChangeShader(contentManager, GLEnums::SHADER_TYPE::FRAGMENT, shaderPath))
                        throw "Oh no";

                    currentLightCull->SetDrawBindData(worldModel->drawBinds);
                    return Argument("Lighting cull mode updated");
                }
                else
                    return Argument("Invalid argument");
            }
                                             , FORCE_STRING_ARGUMENTS::PER_ARGUMENT
                                             , AUTOCOMPLETE_TYPE::ONLY_CUSTOM
                                             , "normal", "adaptive", "normalDebug", "adaptiveDebug", "clustered", "clusteredDebug"
    ));

    console.AddCommand(new CommandCallMethod("snapshot", [&](const std::vector<Argument>& args)
            {
                snapshotCamera = camera;

                return "Camera Set";
            }
    ));

    console.AddCommand(new CommandGetSet<glm::ivec2>("tileToDraw", &tileToDraw));

    console.AddCommand(new CommandCallMethod("msaaCount"
                                             , [&](const std::vector<Argument>& args)
            {
                if(args.size() != 1)
                    return Argument("Needs 1 parameter");

                msaaCount = std::stoi(args.front().value);

                ResizeFramebuffer(screenWidth, screenHeight, true);

                return Argument("msaaCount set to " + std::to_string(msaaCount));
            }
    ));

    console.AddCommand(new CommandCallMethod("camera"
                                             , [&](const std::vector<Argument>& args)
            {
                if(currentCamera == &camera)
                    currentCamera = &snapshotCamera;
                else
                    currentCamera = &camera;

                return Argument("Camera set");
            }
    ));

    guiManager.AddContainer(&console);

    Logger::SetCallOnLog(
            [&](std::string text)
            {
                if(text.back() == '\n')
                    text.pop_back();

                console.AddText(text);
            });
}

void Main::InitInput()
{
    Input::Init(&window, true);

    Input::RegisterKeyCallback(
            [&](const KeyState& keyState)
            {
                if(keyState.action == KEY_ACTION::DOWN)
                {
                    if(!keysDown.count(keyState.key))
                        keysDown.insert(keyState.key);

                    if(keyState.key == KEY_CODE::SECTION)
                    {
                        if(console.GetActive())
                        {
                            Input::LockCursor(screenWidth / 2, screenHeight / 2);
                            console.Deactivate();
                        }
                        else
                        {
                            Input::LockCursor(-1, -1);
                            console.Activate();
                        }
                    }
                    else if(keyState.key == KEY_CODE::F11)
                        dumpPreBackBuffer = true;
                    else if(keyState.key == KEY_CODE::F12)
                        dumpPostBackBuffer = true;
                }
                else if(keyState.action == KEY_ACTION::UP)
                {
                    keysDown.erase(keyState.key);
                }

                guiManager.KeyEvent(keyState);
            });
    Input::RegisterMouseButtonCallback(
            [&](const MouseButtonState& buttonState)
            {
                guiManager.MouseEvent(buttonState);
            });

    Input::RegisterCharCallback(
            [&](int character)
            {
                guiManager.CharEvent(character);
            });

    Input::RegisterScrollCallback(
            [&](int distance)
            {
                guiManager.ScrollEvent(distance);
            });

    window.RegisterFocusGainCallback(
            [&]()
            {
                if(!console.GetActive())
                    Input::LockCursor(screenWidth / 2, screenHeight / 2);
            });

    window.RegisterFocusLossCallback(
            [&]()
            {
                Input::LockCursor(-1, -1);
            });

    window.RegisterWindowSizeChangeCallback(
            [&](int width, int height)
            {
                screenWidth = width;
                screenHeight = height;

                spriteRenderer.SetScreenSize(screenWidth, screenHeight);
                console.SetSize(screenWidth, screenHeight / 2);
                camera.SetPerspectiveVertical(camera.GetFOVVertical(), screenWidth, screenHeight, camera.GetNearPlane(), camera.GetFarPlane());
                snapshotCamera.SetPerspectiveVertical(snapshotCamera.GetFOVVertical(), screenWidth, screenHeight, snapshotCamera.GetNearPlane(), snapshotCamera.GetFarPlane());

                ResizeFramebuffer(width, screenHeight, msaaCount);

                glViewport(0, 0, screenWidth, screenHeight);
            });

    Input::Update();
}

void Main::InitQuieries()
{
    glGenQueries(2, queries);
}

struct LineVertex
{
    glm::vec3 position;
    glm::vec3 color;

    LineVertex()
            : position(0.0f)
              , color(1.0f)
    {}
    LineVertex(glm::vec3 position)
            : position(position)
              , color(1.0f)
    {}
    LineVertex(glm::vec3 position, glm::vec3 color)
            : position(position)
              , color(color)
    {}
};
GLVertexBuffer lineVertexBuffer;
GLIndexBuffer lineIndexBuffer;
GLDrawBinds lineDrawBinds;

bool Main::InitFrameBuffers()
{
    glGenFramebuffers(1, &frameBufferDepthOnly);
    glBindFramebuffer(GL_FRAMEBUFFER, frameBufferDepthOnly);

    ResizeFramebuffer(screenWidth, screenHeight, true);

    GLenum error = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if(error != GL_FRAMEBUFFER_COMPLETE)
        Logger::LogLine(LOG_TYPE::FATAL, "Framebuffer not complete");

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    return error == GL_FRAMEBUFFER_COMPLETE;
}

bool Main::InitShaders()
{
    /*lightCullAdaptive.InitShaderConstants(screenWidth, screenHeight);
    if(!lightCullAdaptive.Init(contentManager, console))
        return false;

    lightCullNormal.InitShaderConstants(screenWidth, screenHeight);
    if(!lightCullNormal.Init(contentManager, console))
        return false;*/

    lightCullClustered = new LightCullClustered();

    currentLightCull = lightCullClustered;

    currentLightCull->InitShaderConstants(screenWidth, screenHeight);
    if(!currentLightCull->Init(contentManager, console))
        return false;
    //currentLightCull = &lightCullNormal;

    ////////////////////////////////////////////////////////////
    // Lines

    const static int MAX_LINES = 1024 * 8;

    lineVertexBuffer.Init<glm::vec3, glm::vec3>(GLEnums::BUFFER_USAGE::STREAM_DRAW, nullptr, sizeof(LineVertex) * MAX_LINES * 2);
    lineIndexBuffer.Init<GLuint>(GLEnums::BUFFER_USAGE::STREAM_DRAW, nullptr, sizeof(LineVertex) * MAX_LINES * 2 * 2);

    lineDrawBinds.AddBuffers(&lineVertexBuffer, &lineIndexBuffer);
    lineDrawBinds.AddShaders(contentManager
                             , GLEnums::SHADER_TYPE::VERTEX, "rendering/line.vert"
                             , GLEnums::SHADER_TYPE::FRAGMENT, "rendering/line.frag");
    lineDrawBinds.AddUniform("viewProjectionMatrix", glm::mat4());
    lineDrawBinds.Init();

    return true;
}

void Main::Update(Timer& deltaTimer)
{
    if(!console.GetActive())
    {
        if(keysDown.count(KEY_CODE::A))
            currentCamera->MoveRight(-cameraSpeed * deltaTimer.GetDeltaMillisecondsFraction());
        else if(keysDown.count(KEY_CODE::D))
            currentCamera->MoveRight(cameraSpeed * deltaTimer.GetDeltaMillisecondsFraction());

        if(keysDown.count(KEY_CODE::W))
            currentCamera->MoveFoward(cameraSpeed * deltaTimer.GetDeltaMillisecondsFraction());
        else if(keysDown.count(KEY_CODE::S))
            currentCamera->MoveFoward(-cameraSpeed * deltaTimer.GetDeltaMillisecondsFraction());

        if(keysDown.count(KEY_CODE::V))
            currentCamera->MoveUp(cameraSpeed * deltaTimer.GetDeltaMillisecondsFraction());
        else if(keysDown.count(KEY_CODE::C))
            currentCamera->MoveUp(-cameraSpeed * deltaTimer.GetDeltaMillisecondsFraction());

        glm::vec2 mouseDelta = Input::GetMouseDelta();

        currentCamera->Rotate(mouseDelta * 0.0025f);
    }

    guiManager.Update(deltaTimer.GetDelta());

    contentManager.HotReload();

    lightManager.Update(deltaTimer, primitiveDrawer);
}

void Main::Render(Timer& deltaTimer)
{
    auto viewMatrix = currentCamera->GetViewMatrix();
    auto viewMatrixInverse = glm::inverse(currentCamera->GetViewMatrix());
    auto projectionMatrix = currentCamera->GetProjectionMatrix();
    auto projectionMatrixInverse = glm::inverse(currentCamera->GetProjectionMatrix());
    auto viewProjectionMatrix = projectionMatrix * viewMatrix;

    primitiveDrawer.sphereBinds["viewProjectionMatrix"] = viewProjectionMatrix;
    worldModel->drawBinds["viewProjectionMatrix"] = viewProjectionMatrix;
    //worldModel->drawBinds["Lights"] = &lightManager.GetLightsBuffer();

    lineDrawBinds["viewProjectionMatrix"] = viewProjectionMatrix;

    // Needed to make hot reloading work
    //glm::ivec2 screenSize(screenWidth, screenHeight);
    //lightCull["ScreenSize"] = screenSize;

    // Set states
    glClearColor(0.2f, 0.2f, 0.5f, 1.0f);
    glClearDepth(0.0f);

    glDisable(GL_BLEND);

    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE);
    glDepthFunc(GL_GREATER);

    // Bind custom framebuffer
    glBindFramebuffer(GL_FRAMEBUFFER, frameBufferDepthOnly);
    glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

    // Light pass
    auto lightCullTime = currentLightCull->TimedDraw(viewMatrix, projectionMatrixInverse, lightManager);

    //worldModel->drawBinds.GetSSBO("TileLights")->Replace(lightCull.GetActiveTileLightsData());

    // Forward pass (opaque)
    glBeginQuery(GL_TIME_ELAPSED, queries[0]);
    worldModel->DrawOpaque();
    glEndQuery(GL_TIME_ELAPSED);

    GLint timeAvailable = 0;
    while(!timeAvailable)
        glGetQueryObjectiv(queries[0],  GL_QUERY_RESULT_AVAILABLE, &timeAvailable);

    GLuint64 opaqueTime;
    glGetQueryObjectui64v(queries[0], GL_QUERY_RESULT, &opaqueTime);

    primitiveDrawer.End();

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glBlendEquation(GL_FUNC_ADD);
    glDepthMask(GL_FALSE);

    // Forward pass (transparent)
    worldModel->DrawTransparent(camera.GetPosition());

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glBindFramebuffer(GL_READ_FRAMEBUFFER, frameBufferDepthOnly);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
    glBlitFramebuffer(0, 0, screenWidth, screenHeight, 0, 0, screenWidth, screenHeight, GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT, GL_NEAREST);

    glBindFramebuffer(GL_READ_FRAMEBUFFER, 0); // Bind 0 for screenshots
    if(dumpPreBackBuffer)
       DumpBackBuffer();

    glDisable(GL_CULL_FACE);

    glDisable(GL_DEPTH_TEST);

    spriteRenderer.Begin();

    std::string frameString = std::to_string(averageFrameTime) + " [" + std::to_string(lastMinFrameTime) + ";" + std::to_string(lastMaxFrameTime) + " ]";
    spriteRenderer.DrawString(characterSet24, frameString, glm::vec2(0.0f, screenHeight - 48));
    spriteRenderer.DrawString(characterSet24, std::to_string(currentFrameTime), glm::vec2(0.0f, screenHeight - 24));


    spriteRenderer.DrawString(characterSet24, "Light cull: " + std::to_string(lightCullTime * 1e-6f), glm::vec2(0.0f, screenHeight - 72));
    spriteRenderer.DrawString(characterSet24, "Opaque: " + std::to_string(opaqueTime * 1e-6f), glm::vec2(0.0f, screenHeight - 96));

    //Logger::LogLine(LOG_TYPE::NONE, std::to_string(lightCullTime * 1e-6f), ", ", std::to_string(opaqueTime * 1e-6f));

    if(drawLightCount)
        currentLightCull->DrawLightCount(spriteRenderer, characterSet8, characterSet24);

    guiManager.Draw(&spriteRenderer);

    spriteRenderer.End();

    if(dumpPostBackBuffer)
        DumpBackBuffer();

    window.SwapBuffers();
}

bool Main::ResizeFramebuffer(int width, int height, bool recreateBuffers)
{
    if(recreateBuffers)
    {
        glBindFramebuffer(GL_FRAMEBUFFER, frameBufferDepthOnly);

        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE, 0, 0);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE, 0, 0);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D_MULTISAMPLE, 0, 0);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, 0, 0);

        glDeleteTextures(1, &depthBufferTexture);
        glDeleteTextures(1, &backBufferTexture);

        depthBufferTexture = 0;
        backBufferTexture = 0;

        glGenTextures(1, &depthBufferTexture);
        glGenTextures(1, &backBufferTexture);
    }

    if(msaaCount < 0)
    {
        Logger::LogLine(LOG_TYPE::DEBUG, "msaaCount < 0, automatically set to 0");
        msaaCount = 0;
    }
    else
    {
        GLint maxColor = 0;
        glGetIntegerv(GL_MAX_COLOR_TEXTURE_SAMPLES, &maxColor);

        GLint maxDepth = 0;
        glGetIntegerv(GL_MAX_DEPTH_TEXTURE_SAMPLES, &maxDepth);

        if(msaaCount > maxColor
                || msaaCount > maxDepth)
        {
            Logger::LogLine(LOG_TYPE::DEBUG, "msaaCount of ", msaaCount, " not supported. ", std::min(maxColor, maxDepth), " will be used");

            msaaCount = std::min(maxColor, maxDepth);
        }
    }

    if(msaaCount != 0)
    {
        glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, depthBufferTexture);
        glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, msaaCount, GL_DEPTH_COMPONENT32, width, height, GL_TRUE);

        glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, backBufferTexture);
        glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, msaaCount, GL_RGBA8, width, height, GL_TRUE);
        glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, 0);

        if(recreateBuffers)
        {
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D_MULTISAMPLE, depthBufferTexture, 0);
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, backBufferTexture, 0);
        }
    }
    else
    {
        glBindTexture(GL_TEXTURE_2D, depthBufferTexture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32, width, height, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, nullptr);

        glBindTexture(GL_TEXTURE_2D, backBufferTexture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
        glBindTexture(GL_TEXTURE_2D, 0);

        if(recreateBuffers)
        {
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthBufferTexture, 0);
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, backBufferTexture, 0);
        }
    }

    if(recreateBuffers)
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

    //lightCullAdaptive.ResolutionChanged(width, height);
    //lightCullNormal.ResolutionChanged(width, height);

    return true;
}

void Main::DumpBackBuffer()
{
    ilInit();

    if(ilGetError() != 0)
    {
        Logger::LogLine(LOG_TYPE::FATAL, "Couldn't dump backbuffer to screenshot");
        return;
    }

    ILuint image;
    ilGenImages(1, &image);

    if(ilGetError() != 0)
    {
        Logger::LogLine(LOG_TYPE::FATAL, "Couldn't dump backbuffer to screenshot");
        return;
    }

    GLubyte* backBufferData = new GLubyte[screenWidth * screenHeight * 3];

    glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
    glReadPixels(0, 0, screenWidth, screenHeight, GL_RGB, GL_UNSIGNED_BYTE, backBufferData);

    ilBindImage(image);
    ilTexImage(screenWidth, screenHeight, 1, 3, IL_RGB, IL_UNSIGNED_BYTE, backBufferData);
    ilSetData(backBufferData);

    if(ilGetError() != 0)
    {
        ilDeleteImage(image);
        delete[] backBufferData;
        Logger::LogLine(LOG_TYPE::FATAL, "Couldn't dump backbuffer to screenshot");
        return;
    }

    std::string fileName = "backbuffer";
    std::string format = ".png";

    int nameCount = 0;
    std::ifstream file((fileName + std::to_string(nameCount) + format).c_str());
    while(file.is_open())
    {
        ++nameCount;
        file.close();
        file.open((fileName + std::to_string(nameCount) + format).c_str());
    }

    ilEnable(IL_FILE_OVERWRITE);
    ilSaveImage((fileName + std::to_string(nameCount) + format).c_str());

    if(ilGetError() != 0)
    {
        ilDeleteImage(image);
        delete[] backBufferData;
        Logger::LogLine(LOG_TYPE::FATAL, "Couldn't dump backbuffer to screenshot");
        return;
    }

    ilDeleteImage(image);
    delete[] backBufferData;
    dumpPreBackBuffer = false;
    dumpPostBackBuffer = false;
}
