#include <chrono>
#include <thread>
#include <set>
#include <glm/gtc/matrix_transform.hpp>

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

    const static int lightCount = 16;

    const float LIGHT_DEFAULT_AMBIENT = 0.1f;
    const float LIGHT_MAX_STRENGTH = 10.0f;
    const float LIGHT_MAX_LIFETIME = 2000.0f;

    struct Lights
    {
        glm::vec3 padding;
        float ambientStrength;
        LightData lights[lightCount];
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

    bool wireframe;

    float averageFrameTime;
    float minFrameTime;
    float maxFrameTime;

    int InitContent();
    void InitConsole();
    void InitInput();
    void InitLights();

    void Update(Timer& deltaTimer);
    void Render(Timer& deltaTimer);
};

int main(int argc, char* argv[])
{
    Main main;
    return main.Run();
}

Main::Main()
        : contentManager("content")
        , wireframe(false)
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

        InitInput();
        InitContent();
        InitConsole();

        // Use a reversed depth buffer
        glDepthRange(1, 0);
        camera.InitFovVertical({0.0f, 0.0f, -5.0f}
                               , {0.0f, 0.0f, 0.0f}
                               , glm::half_pi<float>()
                               , 1280
                               , 720
                               , 100.0f
                               , 0.01f);

        primitiveDrawer.Init(contentManager);

        InitLights();

        double frameTime = 0.0;
        unsigned long frameCount = 0;

        minFrameTime = std::numeric_limits<float>::max();
        maxFrameTime = std::numeric_limits<float>::min();

        Timer deltaTimer;
        deltaTimer.ResetDelta();
        while(window.PollEvents())
        {
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

                minFrameTime = std::numeric_limits<float>::max();
                maxFrameTime = std::numeric_limits<float>::min();
            }

            Update(deltaTimer);
            Render(deltaTimer);

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

    for(int i = 0; i < lightCount; ++i)
    {
        LightData newLight;

        float xPos = ((rand() / (float)RAND_MAX) - 0.5f) * 2.0f * 12.0f;
        float yPos = (rand() / (float)RAND_MAX) * 8.0f + 1.0f;
        float zPos = ((rand() / (float)RAND_MAX) - 0.5f) * 2.0f * 8.0f;

        newLight.position = glm::vec3(xPos, yPos, zPos);
        newLight.color = glm::vec3(rand() / (float)RAND_MAX, rand() / (float)RAND_MAX, rand() / (float)RAND_MAX);
        newLight.padding = rand() / (float)RAND_MAX * LIGHT_MAX_LIFETIME;

        if(newLight.padding <= LIGHT_MAX_LIFETIME * 0.5f)
            newLight.strength = (newLight.padding / (LIGHT_MAX_LIFETIME * 0.5f)) * LIGHT_MAX_STRENGTH;
        else
            newLight.strength = ((LIGHT_MAX_LIFETIME * 0.5f - (newLight.padding - LIGHT_MAX_LIFETIME * 0.5f)) / (LIGHT_MAX_LIFETIME * 0.5f)) * LIGHT_MAX_STRENGTH;

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

    for(int i = 0; i < lightCount; ++i)
    {
        lightsBuffer.lights[i].padding += deltaTimer.GetDeltaMillisecondsFraction();

        float newStrength;
        if(lightsBuffer.lights[i].padding <= LIGHT_MAX_LIFETIME * 0.5f)
            newStrength = (lightsBuffer.lights[i].padding / (LIGHT_MAX_LIFETIME * 0.5f)) * LIGHT_MAX_STRENGTH;
        else
            newStrength = ((LIGHT_MAX_LIFETIME * 0.5f - (lightsBuffer.lights[i].padding - LIGHT_MAX_LIFETIME * 0.5f)) / (LIGHT_MAX_LIFETIME * 0.5f)) * LIGHT_MAX_STRENGTH;


        lightsBuffer.lights[i].strength = newStrength;

        if(lightsBuffer.lights[i].padding >= LIGHT_MAX_LIFETIME)
        {
            lightsBuffer.lights[i].padding = 0.0f;

            float xPos = ((rand() / (float)RAND_MAX) - 0.5f) * 2.0f * 12.0f;
            float yPos = (rand() / (float)RAND_MAX) * 8.0f + 1.0f;
            float zPos = ((rand() / (float)RAND_MAX) - 0.5f) * 2.0f * 8.0f;

            lightsBuffer.lights[i].position = glm::vec3(xPos, yPos, zPos);
            lightsBuffer.lights[i].strength = 0.0f;
            lightsBuffer.lights[i].color = glm::vec3(rand() / (float)RAND_MAX, rand() / (float)RAND_MAX, rand() / (float)RAND_MAX);
        }

        primitiveDrawer.DrawSphere(lightsBuffer.lights[i].position, 0.1f, lightsBuffer.lights[i].color);
    }
}

void Main::Render(Timer& deltaTimer)
{
    primitiveDrawer.sphereBinds["viewProjectionMatrix"] = camera.GetProjectionMatrix() * camera.GetViewMatrix();
    worldModel->drawBinds["viewProjectionMatrix"] = camera.GetProjectionMatrix() * camera.GetViewMatrix();
    worldModel->drawBinds["Lights"] = lightsBuffer;

    glClearColor(0.2f, 0.2f, 0.5f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);
    glCullFace(GL_BACK);
    glDepthMask(GL_TRUE);
    glDisable(GL_BLEND);

    if(wireframe)
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    worldModel->DrawOpaque(camera.GetPosition());

    primitiveDrawer.End();

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glBlendEquation(GL_FUNC_ADD);
    glDepthMask(GL_FALSE);

    worldModel->DrawTransparent(camera.GetPosition());

    glDepthMask(GL_TRUE);

    if(wireframe)
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);

    spriteRenderer.Begin();

    std::string frameString = std::to_string(averageFrameTime) + " [" + std::to_string(minFrameTime) + ";" + std::to_string(maxFrameTime) + " ]";
    spriteRenderer.DrawString(characterSet, frameString, glm::vec2(0.0f, 0.0f));
    spriteRenderer.DrawString(characterSet, std::to_string(currentFrameTime), glm::vec2(0.0f, 16.0f));

    guiManager.Draw(&spriteRenderer);

    spriteRenderer.End();

    window.SwapBuffers();
}
