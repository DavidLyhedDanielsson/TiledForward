#include <chrono>
#include <thread>
#include <set>
#include <glm/gtc/matrix_transform.hpp>
#include <X11/Xlib.h>

#include "window.h"
#include "timer.h"
#include "logger.h"
#include "input.h"
#include "perspectiveCamera.h"
#include "glVertexBuffer.h"
#include "glDrawBinds.h"
#include "texture.h"
#include "contentManager.h"
#include "spriteRenderer.h"
#include "console/guiManager.h"
#include "console/console.h"

#include "libobj.h"
#include "console/commandGetSet.h"
#include "OBJModel.h"
#include "console/colors.h"
#include "glPixelShader.h"
#include "shaderContentParameters.h"
#include "console/commandCallMethod.h"
#include "primitiveDrawer.h"

#include <glm/gtx/component_wise.hpp>

class Main
{
public:
    Main();
    ~Main()
    { }

    int Run();
protected:
private:
    struct LightData
    {
        glm::vec3 position;
        float strength;
        glm::vec3 color;
        float padding;
    };

    const static int LIGHT_COUNT = 2048;

    const float LIGHT_DEFAULT_AMBIENT = 0.1f;
    const float LIGHT_MIN_STRENGTH = 0.0f;
    const float LIGHT_MAX_STRENGTH = 2.0f;
    const float LIGHT_MAX_LIFETIME = 1000.0f;

    const float LIGHT_RANGE_X = 14.0f;
    const float LIGHT_MAX_Y = 8.0f;
    const float LIGHT_RANGE_Z = 6.0f;

    /*const float LIGHT_RANGE_X = 0.0f;
    const float LIGHT_MAX_Y = 0.0f;
    const float LIGHT_RANGE_Z = 0.0f;*/

    const static int DEFAULT_SCREEN_WIDTH = 1280;
    const static int DEFAULT_SCREEN_HEIGHT = 720;

    int screenWidth = DEFAULT_SCREEN_WIDTH;
    int screenHeight = DEFAULT_SCREEN_HEIGHT;

    int WORK_GROUP_WIDTH = 32;
    int WORK_GROUP_HEIGHT = 32;
    int MAX_LIGHTS_PER_TILE = 1024;

    struct Lights
    {
        glm::vec3 padding;
        float ambientStrength;
        LightData lights[LIGHT_COUNT];
    } lightsBuffer;

    OSWindow window;

    std::set<KEY_CODE> keysDown;

    GUIManager guiManager;
    ContentManager contentManager;

    PrimitiveDrawer primitiveDrawer;

    SpriteRenderer spriteRenderer;
    Console console;

    CharacterSet* characterSet;
    OBJModel* worldModel;

    PerspectiveCamera camera;
    PerspectiveCamera snapshotCamera;
    PerspectiveCamera* currentCamera;

    // Debug
    bool wireframe;
    bool drawTiles;
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

    GLDrawBinds lightCull;

    GLuint queries[2];

    int msaaCount;

    int InitContent();
    void InitConsole();
    void InitInput();
    void InitLights();
    bool InitFrameBuffers();
    void InitQuieries();

    void Update(Timer& deltaTimer);
    void Render(Timer& deltaTimer);
    void DrawTiles();
    bool ResizeFramebuffer(int width, int height, bool recreateBuffers);
};

int main(int argc, char* argv[])
{
    Main main;
    return main.Run();
}

Main::Main()
        : contentManager("content")
          , wireframe(false)
          , drawTiles(false)
          , tileToDraw(-1)
          , averageFrameTime(0.0f)
          , msaaCount(2)
          , depthBufferTexture(0)
          , backBufferTexture(0)
          , currentCamera(&camera)
{ }

float currentFrameTime = 0.0f;

int Main::Run()
{
    Logger::ClearLog();

    // TODO: Fix rename of index to blockIndex, e.g. "Used for index rounding" -> "Used for blockIndex rounding"

    if(window.Create((unsigned int)screenWidth, (unsigned int)screenHeight) == OSWindow::NONE)
    {
        if(!InitFrameBuffers())
            return 2;
        InitInput();

        int errVal = InitContent();
        if(errVal != 0)
            return errVal;

        InitConsole();

        // Use a reversed depth buffer
        glClipControl(GL_LOWER_LEFT, GL_ZERO_TO_ONE);
        glDepthRange(1.0, 0.0);
        camera.InitFovVertical({0.0f, 0.0f, -5.0f}
                               , {0.0f, 0.0f, 0.0f}
                               , glm::half_pi<float>()
                               , screenWidth
                               , screenHeight
                               , 0.01f
                               , 100.0f);
        snapshotCamera = camera;

        primitiveDrawer.Init(contentManager);

        InitLights();
        InitQuieries();

        double frameTime = 0.0;
        unsigned long frameCount = 0;

        minFrameTime = std::numeric_limits<float>::max();
        maxFrameTime = std::numeric_limits<float>::min();

        const static int FRAME_CAP = 60;

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

    characterSet = contentManager.Load<CharacterSet>("UbuntuMono-R24.ttf");

    worldModel = contentManager.Load<OBJModel>("sponza.obj");
    if(worldModel == nullptr)
        return 3;

    worldModel->drawBinds["Lights"] = lightCull["Lights"];
    worldModel->drawBinds["LightIndices"] = lightCull["LightIndices"];
    worldModel->drawBinds["TileLights"] = lightCull["TileLights"];
    worldModel->drawBinds["ScreenSize"] = glm::ivec2(screenWidth, screenHeight);

    return 0;
}

void Main::InitConsole()
{
    console.Init(&contentManager
                 , Rect(0.0f, 0.0f, screenWidth, screenHeight / 2.0f)
                 , console.GenerateDoomStyle(&contentManager, characterSet)
                 , console.GenerateDoomStyleBackgroundStyle(&contentManager)
                 , false
                 , false
                 , false
                 , false);
    console.Autoexec();
    console.AddCommand(new CommandGetSet<bool>("wireframe", &wireframe));

    console.AddCommand(new CommandCallMethod("light_position"
                                             , [&](const std::vector<Argument>& args)
            {
                if(args.size() != 1)
                    return Argument("Needs 1 parameter");

                glm::vec3 newPosition = camera.GetPosition();

                lightsBuffer.lights[std::stoi(args.front().value)].position = newPosition;

                Argument returnArgument;
                newPosition >> returnArgument;
                returnArgument.value.insert(0, "Position set to ");

                return returnArgument;
            }
    ));
    console.AddCommand(new CommandCallMethod("light_strength"
                                             , [&](const std::vector<Argument>& args)
            {
                if(args.size() != 2)
                    return Argument("Needs 2 parameter");

                float newStrength = std::stof(args.back().value);

                lightsBuffer.lights[std::stoi(args.front().value)].strength = newStrength;

                Argument returnArgument;
                newStrength >> returnArgument;
                returnArgument.value.insert(0, "Strength set to ");

                return returnArgument;
            }
    ));
    //console.AddCommand(new CommandGetSet<float>("light_lightStrength", &worldModel->lightData.lightStrength));
    console.AddCommand(new CommandGetSet<float>("light_ambientStrength", &lightsBuffer.ambientStrength));

    console.AddCommand(new CommandCallMethod("snapshot", [&](const std::vector<Argument>& args)
            {
                snapshotCamera = camera;

                return "Camera Set";
            }
    ));

    console.AddCommand(new CommandGetSet<bool>("drawTiles", &drawTiles));
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

void Main::InitLights()
{
    lightsBuffer.padding = glm::vec3(1.0f, 1.2f, 1.23f);
    lightsBuffer.ambientStrength = LIGHT_DEFAULT_AMBIENT;

    for(int i = 0; i < LIGHT_COUNT; ++i)
    {
        LightData newLight;

        float xPos = ((rand() / (float)RAND_MAX) - 0.5f) * 2.0f * LIGHT_RANGE_X;
        float yPos = (rand() / (float)RAND_MAX) * LIGHT_MAX_Y;
        float zPos = ((rand() / (float)RAND_MAX) - 0.5f) * 2.0f * LIGHT_RANGE_Z;

        newLight.position = glm::vec3(xPos, yPos, zPos);
        newLight.color = glm::vec3(rand() / (float)RAND_MAX, rand() / (float)RAND_MAX, rand() / (float)RAND_MAX);
        newLight.padding = rand() / (float)RAND_MAX * LIGHT_MAX_LIFETIME;

        if(newLight.padding <= LIGHT_MAX_LIFETIME * 0.5f)
            newLight.strength = (newLight.padding / (LIGHT_MAX_LIFETIME * 0.5f)) * (LIGHT_MAX_STRENGTH - LIGHT_MIN_STRENGTH) + LIGHT_MIN_STRENGTH;
        else
            newLight.strength = ((LIGHT_MAX_LIFETIME * 0.5f - (newLight.padding - LIGHT_MAX_LIFETIME * 0.5f)) / (LIGHT_MAX_LIFETIME * 0.5f)) * (LIGHT_MAX_STRENGTH - LIGHT_MIN_STRENGTH) + LIGHT_MIN_STRENGTH;

        lightsBuffer.lights[i] = newLight;
    }

    lightCull["Lights"] = lightsBuffer;
    //worldModel->drawBinds["Lights"] = lightsBuffer; // Allocate data before runtime
}

void Main::Update(Timer& deltaTimer)
{
    if(!console.GetActive())
    {
        if(keysDown.count(KEY_CODE::A))
            currentCamera->MoveRight(-0.01f * deltaTimer.GetDeltaMillisecondsFraction());
        else if(keysDown.count(KEY_CODE::D))
            currentCamera->MoveRight(0.01f * deltaTimer.GetDeltaMillisecondsFraction());

        if(keysDown.count(KEY_CODE::W))
            currentCamera->MoveFoward(0.01f * deltaTimer.GetDeltaMillisecondsFraction());
        else if(keysDown.count(KEY_CODE::S))
            currentCamera->MoveFoward(-0.01f * deltaTimer.GetDeltaMillisecondsFraction());

        if(keysDown.count(KEY_CODE::V))
            currentCamera->MoveUp(0.01f * deltaTimer.GetDeltaMillisecondsFraction());
        else if(keysDown.count(KEY_CODE::C))
            currentCamera->MoveUp(-0.01f * deltaTimer.GetDeltaMillisecondsFraction());

        glm::vec2 mouseDelta = Input::GetMouseDelta();

        currentCamera->Rotate(mouseDelta * 0.0025f);
    }

    guiManager.Update(deltaTimer.GetDelta());

    contentManager.HotReload();

    for(int i = 0; i < LIGHT_COUNT; ++i)
    {
        lightsBuffer.lights[i].padding += deltaTimer.GetDeltaMillisecondsFraction();

        float newStrength;
        if(lightsBuffer.lights[i].padding <= LIGHT_MAX_LIFETIME * 0.5f)
            newStrength = (lightsBuffer.lights[i].padding / (LIGHT_MAX_LIFETIME * 0.5f)) * (LIGHT_MAX_STRENGTH - LIGHT_MIN_STRENGTH) + LIGHT_MIN_STRENGTH;
        else
            newStrength = ((LIGHT_MAX_LIFETIME * 0.5f - (lightsBuffer.lights[i].padding - LIGHT_MAX_LIFETIME * 0.5f)) / (LIGHT_MAX_LIFETIME * 0.5f)) * (LIGHT_MAX_STRENGTH - LIGHT_MIN_STRENGTH) + LIGHT_MIN_STRENGTH;


        lightsBuffer.lights[i].strength = newStrength;

        if(lightsBuffer.lights[i].padding >= LIGHT_MAX_LIFETIME)
        {
            lightsBuffer.lights[i].padding = 0.0f;

            float xPos = ((rand() / (float)RAND_MAX) - 0.5f) * 2.0f * LIGHT_RANGE_X;
            float yPos = (rand() / (float)RAND_MAX) * LIGHT_MAX_Y;
            float zPos = ((rand() / (float)RAND_MAX) - 0.5f) * 2.0f * LIGHT_RANGE_Z;

            lightsBuffer.lights[i].position = glm::vec3(xPos, yPos, zPos);
            lightsBuffer.lights[i].strength = 0.0f;
            lightsBuffer.lights[i].color = glm::vec3(rand() / (float)RAND_MAX, rand() / (float)RAND_MAX, rand() / (float)RAND_MAX);
        }

        primitiveDrawer.DrawSphere(lightsBuffer.lights[i].position, lightsBuffer.lights[i].strength, lightsBuffer.lights[i].color);
    }
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

void Main::Render(Timer& deltaTimer)
{
    auto viewMatrix = currentCamera->GetViewMatrix();
    auto viewMatrixInverse = glm::inverse(currentCamera->GetViewMatrix());
    auto projectionMatrix = currentCamera->GetProjectionMatrix();
    auto projectionMatrixInverse = glm::inverse(currentCamera->GetProjectionMatrix());
    auto viewProjectionMatrix = projectionMatrix * viewMatrix;

    primitiveDrawer.sphereBinds["viewProjectionMatrix"] = viewProjectionMatrix;
    worldModel->drawBinds["viewProjectionMatrix"] = viewProjectionMatrix;
    worldModel->drawBinds["Lights"] = lightsBuffer;

    lineDrawBinds["viewProjectionMatrix"] = viewProjectionMatrix;

    lightCull["viewMatrix"] = viewMatrix;
    lightCull["projectionInverseMatrix"] = projectionMatrixInverse;

    lightCull["Lights"] = lightsBuffer;

    int zero = 0;
    lightCull.GetSSBO("LightIndices")->UpdateData(0, &zero, sizeof(int));

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
//#define DRAW_TO_CUSTOM
#ifdef DRAW_TO_CUSTOM
    glBindFramebuffer(GL_FRAMEBUFFER, frameBufferDepthOnly);
#else
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
#endif
    glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

#ifdef DRAW_TO_CUSTOM
    worldModel->DrawOpaque();
#endif

    lightCull.Bind();

    glBindImageTexture(0, backBufferTexture, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA8);

    glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, depthBufferTexture);

    glDispatchCompute((GLuint)std::ceil(screenWidth / (float)WORK_GROUP_WIDTH), (GLuint)std::ceil(screenHeight / (float)WORK_GROUP_HEIGHT), 1);
    lightCull.Unbind();

#ifdef DRAW_TO_CUSTOM
    glBindFramebuffer(GL_READ_FRAMEBUFFER, frameBufferDepthOnly);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
    glBlitFramebuffer(0, 0, screenWidth, screenHeight, 0, 0, screenWidth, screenHeight, GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT, GL_NEAREST);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glBlendEquation(GL_FUNC_ADD);
    glDepthMask(GL_FALSE);

    worldModel->DrawTransparent(camera.GetPosition());
#else
    worldModel->DrawOpaque();

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glBlendEquation(GL_FUNC_ADD);
    glDepthMask(GL_FALSE);

    worldModel->DrawTransparent(camera.GetPosition());
#endif

    glDisable(GL_CULL_FACE);

    if(drawTiles)
        DrawTiles();

    glDisable(GL_DEPTH_TEST);

    spriteRenderer.Begin();

    std::string frameString = std::to_string(averageFrameTime) + " [" + std::to_string(lastMinFrameTime) + ";" + std::to_string(lastMaxFrameTime) + " ]";
    spriteRenderer.DrawString(characterSet, frameString, glm::vec2(0.0f, screenHeight - 48));
    spriteRenderer.DrawString(characterSet, std::to_string(currentFrameTime), glm::vec2(0.0f, screenHeight - 24));

    guiManager.Draw(&spriteRenderer);

    spriteRenderer.End();

    window.SwapBuffers();
}

bool Main::InitFrameBuffers()
{
    const static int MAX_LINES = 1024 * 8;

    lineVertexBuffer.Init<glm::vec3, glm::vec3>(GLEnums::BUFFER_USAGE::STREAM_DRAW, nullptr, sizeof(LineVertex) * MAX_LINES * 2);
    lineIndexBuffer.Init<GLuint>(GLEnums::BUFFER_USAGE::STREAM_DRAW, nullptr, sizeof(LineVertex) * MAX_LINES * 2 * 2);

    lineDrawBinds.AddBuffers(&lineVertexBuffer, &lineIndexBuffer);
    lineDrawBinds.AddShaders(contentManager
                             , GLEnums::SHADER_TYPE::VERTEX, "line.vert"
                             , GLEnums::SHADER_TYPE::FRAGMENT, "line.frag");
    lineDrawBinds.AddUniform("viewProjectionMatrix", glm::mat4());
    lineDrawBinds.Init();

    lightCull.AddUniform("viewMatrix", glm::mat4());
    lightCull.AddUniform("projectionInverseMatrix", glm::mat4());

    ShaderContentParameters parameters;
    parameters.type = GLEnums::SHADER_TYPE::COMPUTE;
    parameters.variables.push_back(std::make_pair("WORK_GROUP_WIDTH", std::to_string(WORK_GROUP_WIDTH)));
    parameters.variables.push_back(std::make_pair("WORK_GROUP_HEIGHT", std::to_string(WORK_GROUP_HEIGHT)));
    parameters.variables.push_back(std::make_pair("MAX_LIGHTS_PER_TILE", std::to_string(MAX_LIGHTS_PER_TILE)));
    lightCull.AddShaders(contentManager, parameters, "lightCull.comp");
    lightCull.Init();

    glm::ivec2 screenSize(screenWidth, screenHeight);
    lightCull["ScreenSize"] = screenSize;

    std::vector<int> data(1 + 40 * 23 * MAX_LIGHTS_PER_TILE);
    data[0] = 0;
    lightCull["LightIndices"] = data;

    data.clear();
    data.resize(40 * 23 * 4, -1);
    lightCull["TileLights"] = data;

    glGenFramebuffers(1, &frameBufferDepthOnly);
    glBindFramebuffer(GL_FRAMEBUFFER, frameBufferDepthOnly);

    ResizeFramebuffer(screenWidth, screenHeight, true);

    GLenum error = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if(error != GL_FRAMEBUFFER_COMPLETE)
        Logger::LogLine(LOG_TYPE::FATAL, "Framebuffer not complete");

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    return error == GL_FRAMEBUFFER_COMPLETE;
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

    lightCull["ScreenSize"] = glm::ivec2(width, height);

    return true;
}

void Main::InitQuieries()
{
    glGenQueries(2, queries);
}

void Main::DrawTiles()
{
    std::vector<LineVertex> linePositions;
    std::vector<GLuint> lineIndices;

    const static glm::ivec2 offsets[4] =
            {
                    glm::ivec2(0, 0)
                    , glm::ivec2(1, 0)
                    , glm::ivec2(1, 1)
                    , glm::ivec2(0, 1)
            };

    auto startX = tileToDraw.x == -1 ? 0 : tileToDraw.x;
    auto endX = tileToDraw.x == -1 ? (int)std::ceil(screenWidth / (float)WORK_GROUP_WIDTH) : tileToDraw.x + 1;

    auto startY = tileToDraw.y == -1 ? 0 : tileToDraw.y;
    auto endY = tileToDraw.y == -1 ? (int)std::ceil(screenHeight / (float)WORK_GROUP_HEIGHT) : tileToDraw.y + 1;
    for(int y = startY; y < endY; ++y)
    {
        for(int x = startX; x < endX; ++x)
        {
            const glm::ivec2 gl_WorkGroupID(x, y);
            const glm::ivec2 gl_WorkGroupSize(WORK_GROUP_WIDTH, WORK_GROUP_HEIGHT);

            const glm::vec2 SCREEN_SIZE(screenWidth, screenHeight);

            glm::vec3 viewPositions[4];

            glm::mat4 projectionInverseMatrix = glm::inverse(snapshotCamera.GetProjectionMatrix());
            glm::mat4 viewInverseMatrix = glm::inverse(snapshotCamera.GetViewMatrix());

            for(int i = 0; i < 4; ++i)
            {
                glm::vec3 ndcPosition = glm::vec3(glm::vec2((gl_WorkGroupID + offsets[i]) * gl_WorkGroupSize) / SCREEN_SIZE, 1.0f);
                ndcPosition.x *= 2.0f;
                ndcPosition.x -= 1.0f;
                ndcPosition.y *= -2.0f;
                ndcPosition.y += 1.0f;

                glm::vec4 unprojectedPosition = projectionInverseMatrix * glm::vec4(ndcPosition, 1.0f);
                unprojectedPosition /= unprojectedPosition.w;

                viewPositions[i] = glm::vec3(unprojectedPosition);
            }

            for(int i = 0; i < 4; ++i)
            {
                glm::vec3 eye(viewInverseMatrix * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));
                glm::vec3 target(viewInverseMatrix * glm::vec4(viewPositions[i], 1.0f));

                linePositions.push_back(LineVertex(eye));
                linePositions.push_back(LineVertex(target));

                lineIndices.push_back(lineIndices.size());
                lineIndices.push_back(lineIndices.size());
            }
        }
    }

    lineVertexBuffer.Update(linePositions.data(), linePositions.size() * sizeof(linePositions[0]));
    lineIndexBuffer.Update(lineIndices);

    lineDrawBinds.Bind();
    lineDrawBinds.DrawElements(GLEnums::DRAW_MODE::LINES);
    lineDrawBinds.Unbind();
}
