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

    const static int LIGHT_COUNT = 1;

    const float LIGHT_DEFAULT_AMBIENT = 0.1f;
    const float LIGHT_MIN_STRENGTH = 2.5f;
    const float LIGHT_MAX_STRENGTH = 2.5f;
    const float LIGHT_MAX_LIFETIME = 5000.0f;

    /*const float LIGHT_RANGE_X = 12.0f;
    const float LIGHT_MAX_Z = 8.0f;
    const float LIGHT_RANGE_Z = 8.0f;*/

    const float LIGHT_RANGE_X = 0.0f;
    const float LIGHT_MAX_Z = 0.0f;
    const float LIGHT_RANGE_Z = 0.0f;

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

    GLDrawBinds tiles;

    GLuint queries[2];

    int InitContent();
    void InitConsole();
    void InitInput();
    void InitLights();
    bool InitFrameBuffers();
    void InitQuieries();

    void Update(Timer& deltaTimer);
    void Render(Timer& deltaTimer);
    void DrawTiles();
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
{ }

float currentFrameTime = 0.0f;

int Main::Run()
{
    Logger::ClearLog();

    // TODO: Fix rename of index to blockIndex, e.g. "Used for index rounding" -> "Used for blockIndex rounding"

    if(window.Create(1280, 720) == OSWindow::NONE)
    {
        glewExperimental = true;
        if(glewInit() != GLEW_OK)
            return 1;

        if(!InitFrameBuffers())
            return 2;
        InitInput();
        InitContent();
        InitConsole();

        // Use a reversed depth buffer
        glClipControl(GL_LOWER_LEFT, GL_ZERO_TO_ONE);
        glDepthRange(1.0, 0.0);
        camera.InitFovVertical({0.0f, 0.0f, -5.0f}
                               , {0.0f, 0.0f, 0.0f}
                               , glm::half_pi<float>()
                               , 1280
                               , 720
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
    if(!spriteRenderer.Init(contentManager, 1280, 720))
    {
        Logger::LogLine(LOG_TYPE::FATAL, "Couldn't initialize sprite renderer");
        return 1;
    }

    characterSet = contentManager.Load<CharacterSet>("UbuntuMono-R24.ttf");

    worldModel = contentManager.Load<OBJModel>("sponza.obj");
    if(worldModel == nullptr)
        return 3;
}

void Main::InitConsole()
{
    console.Init(&contentManager, Rect(0.0f, 0.0f, 1280.0f, 360.0f), console.GenerateDoomStyle(&contentManager, characterSet), console.GenerateDoomStyleBackgroundStyle(&contentManager), false, false, false, false);
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


    guiManager.AddContainer(&console);

    /*Logger::SetCallOnLog(
            [&](std::string text)
            {
                if(text.back() == '\n')
                    text.pop_back();

                console.AddText(text);
            });*/
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
                            Input::LockCursor(1280 / 2, 720 / 2);
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
                    Input::LockCursor(1280 / 2, 720 / 2);
            });

    window.RegisterFocusLossCallback(
            [&]()
            {
                Input::LockCursor(-1, -1);
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
        float yPos = (rand() / (float)RAND_MAX) * LIGHT_MAX_Z + 1.0f;
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

    worldModel->drawBinds["Lights"] = lightsBuffer; // Allocate data before runtime
}

void Main::Update(Timer& deltaTimer)
{
    if(!console.GetActive())
    {
        if(keysDown.count(KEY_CODE::A))
            camera.MoveRight(-0.01f * deltaTimer.GetDeltaMillisecondsFraction());
        else if(keysDown.count(KEY_CODE::D))
            camera.MoveRight(0.01f * deltaTimer.GetDeltaMillisecondsFraction());

        if(keysDown.count(KEY_CODE::W))
            camera.MoveFoward(0.01f * deltaTimer.GetDeltaMillisecondsFraction());
        else if(keysDown.count(KEY_CODE::S))
            camera.MoveFoward(-0.01f * deltaTimer.GetDeltaMillisecondsFraction());

        if(keysDown.count(KEY_CODE::V))
            camera.MoveUp(0.01f * deltaTimer.GetDeltaMillisecondsFraction());
        else if(keysDown.count(KEY_CODE::C))
            camera.MoveUp(-0.01f * deltaTimer.GetDeltaMillisecondsFraction());

        glm::vec2 mouseDelta = Input::GetMouseDelta();

        camera.Rotate(mouseDelta * 0.0025f);
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
            float yPos = (rand() / (float)RAND_MAX) * LIGHT_MAX_Z + 1.0f;
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
    primitiveDrawer.sphereBinds["viewProjectionMatrix"] = camera.GetProjectionMatrix() * camera.GetViewMatrix();
    worldModel->drawBinds["viewProjectionMatrix"] = camera.GetProjectionMatrix() * camera.GetViewMatrix();
    worldModel->drawBinds["Lights"] = lightsBuffer;

    lineDrawBinds["viewProjectionMatrix"] = camera.GetProjectionMatrix() * camera.GetViewMatrix();

    tiles["viewMatrix"] = camera.GetViewMatrix();
    tiles["projectionMatrix"] = camera.GetProjectionMatrix();
    tiles["projectionInverseMatrix"] = glm::inverse(camera.GetProjectionMatrix());
    tiles["viewInverseMatrix"] = glm::inverse(camera.GetViewMatrix());
    tiles["Lights"] = lightsBuffer;

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

    // Do z-prepass
    //tiles.Bind();
    worldModel->DrawOpaque(camera.GetPosition());

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glBlendEquation(GL_FUNC_ADD);
    glDepthMask(GL_FALSE);

    worldModel->DrawTransparent(camera.GetPosition());

    glDepthMask(GL_TRUE);
    glDisable(GL_BLEND);


    tiles.Bind();

    //glBindTexture(GL_TEXTURE_2D, depthBufferTexture);
    //glUniform1i(0, 0);

    glBindImageTexture(1, backBufferTexture, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA8);
    glUniform1i(1, 1);

    glDispatchCompute((GLuint)std::ceil(1280 / 32.0f), (GLuint)std::ceil(720 / 32.0f), 1);
    tiles.Unbind();

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glBindFramebuffer(GL_READ_FRAMEBUFFER, frameBufferDepthOnly);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
    glBlitFramebuffer(0, 0, 1280, 720, 0, 0, 1280, 720, GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT, GL_NEAREST);

    //tiles.Unbind();

    //glBindFramebuffer(GL_READ_FRAMEBUFFER, depthRenderBuffer);
    //glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);

    // Bind default framebuffer
    /*glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glDepthFunc(GL_LESS);
    glClear(GL_COLOR_BUFFER_BIT);

    if(wireframe)
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    worldModel->DrawOpaque(camera.GetPosition());
    //primitiveDrawer.End();

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glBlendEquation(GL_FUNC_ADD);
    glDepthMask(GL_FALSE);

    worldModel->DrawTransparent(camera.GetPosition());

    glDepthMask(GL_TRUE);

    if(wireframe)
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    //glBindFramebuffer(GL_READ_FRAMEBUFFER, depthRenderBuffer);
    //glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);

    //glBlitFramebuffer(0, 0, 1280, 720, 0, 0, 1280, 720, GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT, GL_NEAREST);*/

    //glDisable(GL_DEPTH_TEST);
    //glDisable(GL_CULL_FACE);

    std::vector<LineVertex> linePositions;
    std::vector<GLuint> lineIndices;

    const int TOP_LEFT = 0;
    const int TOP_RIGHT = 1;
    const int BOTTOM_RIGHT = 2;
    const int BOTTOM_LEFT = 3;

    //primitiveDrawer.DrawSphere(glm::vec3(0.0f), 0.1f, glm::vec3(1.0f));
    primitiveDrawer.End();

    if(drawTiles)
        DrawTiles();

    //if(!drawTiles)
    //{
        const static glm::ivec2 offsets[4] =
                {
                        glm::ivec2(0, 0)
                        , glm::ivec2(1, 0)
                        , glm::ivec2(1, 1)
                        , glm::ivec2(0, 1)
                };

        auto startX = tileToDraw.x == -1 ? 0 : tileToDraw.x;
        auto endX = tileToDraw.x == -1 ? 40 : tileToDraw.x + 1;

        auto startY = tileToDraw.y == -1 ? 0 : tileToDraw.y;
        auto endY = tileToDraw.y == -1 ? 23 : tileToDraw.y + 1;

        for(int y = startY; y < endY; ++y)
        {
            for(int x = startX; x < endX; ++x)
            {
                const glm::ivec2 gl_WorkGroupID(x, y);
                const glm::ivec2 gl_WorkGroupSize(32, 32);

                const glm::vec2 SCREEN_SIZE(1280.0f, 720.0f);

                glm::vec3 viewPositions[4];

                glm::mat4 projectionInverseMatrix = glm::inverse(snapshotCamera.GetProjectionMatrix());
                glm::mat4 viewInverseMatrix = glm::inverse(snapshotCamera.GetViewMatrix());

                for(int i = 0; i < 4; ++i)
                {
                    glm::vec3 ndcPosition = glm::vec3(glm::vec2((gl_WorkGroupID + offsets[i]) * gl_WorkGroupSize)
                                                      / SCREEN_SIZE, 1.0f);
                    ndcPosition.x *= 2.0f;
                    ndcPosition.x -= 1.0f;
                    ndcPosition.y *= -2.0f;
                    ndcPosition.y += 1.0f;

                    glm::vec4 unprojectedPosition = projectionInverseMatrix * glm::vec4(ndcPosition, 1.0f);
                    unprojectedPosition /= unprojectedPosition.w;

                    viewPositions[i] = glm::vec3(unprojectedPosition);
                }

                auto CreatePlane = [](glm::vec3 far0, glm::vec3 far1) -> glm::vec4
                {
                    glm::vec3 planeABC;
                    planeABC = normalize(cross(far0, far1));
                    float dist = dot(far0, planeABC);

                    return glm::vec4(planeABC, dist);
                };

                const int PLANE_TOP = 0;
                const int PLANE_BOTTOM = 1;
                const int PLANE_LEFT = 2;
                const int PLANE_RIGHT = 3;

                glm::vec4 planes[4];
                planes[PLANE_TOP] = CreatePlane(viewPositions[TOP_LEFT], viewPositions[TOP_RIGHT]);
                planes[PLANE_BOTTOM] = CreatePlane(viewPositions[BOTTOM_RIGHT], viewPositions[BOTTOM_LEFT]);
                planes[PLANE_RIGHT] = CreatePlane(viewPositions[TOP_RIGHT], viewPositions[BOTTOM_RIGHT]);
                planes[PLANE_LEFT] = CreatePlane(viewPositions[BOTTOM_LEFT], viewPositions[TOP_LEFT]);

                glm::vec3 zeroPos = glm::vec3(snapshotCamera.GetViewMatrix() * glm::vec4(lightsBuffer.lights[0].position, 1.0f));

                bool inside = true;
                for(int i = 0; i < 4; ++i)
                {
                    float dist = glm::dot(zeroPos, glm::vec3(planes[i])) + planes[i].w;
                    if(dist > lightsBuffer.lights[0].strength)
                        inside = false;
                }

                if(inside)
                {

                    if(zeroPos.z < 0)
                        inside = false;
                }

                glm::vec3 colors[4];
                colors[PLANE_TOP] = glm::vec3(0.0f, 1.0f, 0.0f);
                colors[PLANE_BOTTOM] = glm::vec3(0.0f, 0.0f, 1.0f);
                colors[PLANE_LEFT] = glm::vec3(1.0f, 1.0f, 0.0f);
                colors[PLANE_RIGHT] = glm::vec3(1.0f, 0.0f, 0.0f);

                if(inside)
                for(int i = 0; i < 4; ++i)
                {
                    glm::vec3 eye(viewInverseMatrix * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));
                    glm::vec3 target(viewInverseMatrix * glm::vec4(glm::vec3(viewPositions[i]), 1.0f));

                    glm::vec3 color = inside ? glm::vec3(0.0f, 1.0f, 0.0f) : glm::vec3(1.0f, 0.0f, 0.0f);

                    linePositions.push_back(LineVertex(eye, color));
                    linePositions.push_back(LineVertex(target, color));

                    lineIndices.push_back(lineIndices.size());
                    lineIndices.push_back(lineIndices.size());
                }
            }
        }
    //}

    lineVertexBuffer.Update(linePositions.data(), linePositions.size() * sizeof(linePositions[0]));
    lineIndexBuffer.Update(lineIndices);

    lineDrawBinds.Bind();
    lineDrawBinds.DrawElements(GLEnums::DRAW_MODE::LINES);
    lineDrawBinds.Unbind();

    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);

    spriteRenderer.Begin();

    std::string frameString = std::to_string(averageFrameTime) + " [" + std::to_string(lastMinFrameTime) + ";" + std::to_string(lastMaxFrameTime) + " ]";
    spriteRenderer.DrawString(characterSet, frameString, glm::vec2(0.0f, 720 - 48));
    spriteRenderer.DrawString(characterSet, std::to_string(currentFrameTime), glm::vec2(0.0f, 720 - 24));

    guiManager.Draw(&spriteRenderer);

    spriteRenderer.End();

    window.SwapBuffers();
}

bool Main::InitFrameBuffers()
{
    lineVertexBuffer.Init<glm::vec3, glm::vec3>(GLEnums::BUFFER_USAGE::STREAM_DRAW, nullptr, 8192);
    lineIndexBuffer.Init<GLuint>(GLEnums::BUFFER_USAGE::STREAM_DRAW, nullptr, 8192 * 2);

    lineDrawBinds.AddBuffers(&lineVertexBuffer, &lineIndexBuffer);
    lineDrawBinds.AddShaders(contentManager
                             , GLEnums::SHADER_TYPE::VERTEX, "line.vert"
                             , GLEnums::SHADER_TYPE::FRAGMENT, "line.frag");
    lineDrawBinds.AddUniform("viewProjectionMatrix", glm::mat4());
    lineDrawBinds.Init();

    tiles.AddUniform("viewMatrix", glm::mat4());
    tiles.AddUniform("projectionMatrix", glm::mat4());
    tiles.AddUniform("projectionInverseMatrix", glm::mat4());
    tiles.AddUniform("viewInverseMatrix", glm::mat4());
    tiles.AddUniform("worldMatrix", glm::scale(glm::mat4(), glm::vec3(0.01f)));
    tiles.AddShaders(contentManager, GLEnums::SHADER_TYPE::COMPUTE, "tiles.comp");
    tiles.Init();

    // Depth buffer
    glGenTextures(1, &depthBufferTexture);
    glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, depthBufferTexture);
    //glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32F, 1280, 720, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexStorage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, 8, GL_DEPTH_COMPONENT32, 1280, 720, GL_TRUE);
    glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, 0);

    // Back buffer
    glGenTextures(1, &backBufferTexture);
    glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, backBufferTexture);
    //glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, 1280, 720, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexStorage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, 8 , GL_RGBA8, 1280, 720, GL_TRUE);
    glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, 0);

    glGenFramebuffers(1, &frameBufferDepthOnly);
    glBindFramebuffer(GL_FRAMEBUFFER, frameBufferDepthOnly);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D_MULTISAMPLE, depthBufferTexture, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, backBufferTexture, 0);

    GLenum error = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if(error != GL_FRAMEBUFFER_COMPLETE)
        Logger::LogLine(LOG_TYPE::FATAL, "Framebuffer not complete");

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    auto err = glGetError();

    return error == GL_FRAMEBUFFER_COMPLETE;
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
    auto endX = tileToDraw.x == -1 ? 40 : tileToDraw.x + 1;

    auto startY = tileToDraw.y == -1 ? 0 : tileToDraw.y;
    auto endY = tileToDraw.y == -1 ? 23 : tileToDraw.y + 1;

    for(int y = startY; y < endY; ++y)
    {
        for(int x = startX; x < endX; ++x)
        {
            const glm::ivec2 gl_WorkGroupID(x, y);
            const glm::ivec2 gl_WorkGroupSize(32, 32);

            const glm::vec2 SCREEN_SIZE(1280.0f, 720.0f);

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
