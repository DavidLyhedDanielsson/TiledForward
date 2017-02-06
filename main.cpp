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

//Cool shit!
//#ifdef _WIN32
//#define IF_WINDOWS(...) __VA_ARGS__
//#else
//#define IF_WINDOWS(...)
//#endif

//#ifdef _WIN32

//#include <windows.h>

//int WINAPI WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int nShowCmd)
//{
//#else
int main(int argc, char* argv[])
{
//#endif

    //IF_WINDOWS(Logger::Init());
    Logger::ClearLog();

    // TODO: Fix rename of index to blockIndex, e.g. "Used for index rounding" -> "Used for blockIndex rounding"

    OSWindow window;
    if(window.Create(1280, 720) == OSWindow::NONE)
    {
        //IF_WINDOWS(_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF));

        glewExperimental = true;
        if(glewInit() != GLEW_OK)
            return 1;

        Timer timer;
        timer.UpdateDelta();

        Input::Init(&window, true);

        // Uses a reversed depth buffer
        PerspectiveCamera camera;
        camera.InitFovVertical({0.0f, 0.0f, -5.0f}
                               , {0.0f, 0.0f, 0.0f}
                               , glm::half_pi<float>()
                               , 1280
                               , 720
                               , 100.0f
                               , 0.01f);
        glDepthRange(1, -1);

        std::set<KEY_CODE> keysDown;

        GUIManager guiManager;
        ContentManager contentManager("content");

        // Creates "whiteTexture", so should be first
        SpriteRenderer spriteRenderer;
        if(!spriteRenderer.Init(contentManager, 1280, 720))
        {
            Logger::LogLine(LOG_TYPE::FATAL, "Couldn't initialize sprite renderer");
            return 1;
        }

        CharacterSet* characterSet = contentManager.Load<CharacterSet>("UbuntuMono-R24.ttf");

        OBJModel* worldModel = contentManager.Load<OBJModel>("sponza.obj");
        if(worldModel == nullptr)
            return 3;

        Console console;
        console.Init(&contentManager, Rect(0.0f, 0.0f, 1280.0f, 360.0f), console.GenerateDoomStyle(&contentManager, characterSet), console.GenerateDoomStyleBackgroundStyle(&contentManager), false, false, false, false);
        console.Autoexec();

        bool wireframe = false;
        console.AddCommand(new CommandGetSet<bool>("wireframe", &wireframe));

        console.AddCommand(new CommandCallMethod("light_position"
                                                 , [&](const std::vector<Argument>&)
                {
                    glm::vec3 newPosition = camera.GetPosition();

                    worldModel->lightData.lightPosition = newPosition;

                    worldModel->drawBinds["LightData"] = worldModel->lightData;

                    Argument returnArgument;
                    newPosition >> returnArgument;
                    returnArgument.value.insert(0, "Position set to ");

                    return returnArgument;
                }
        ));
        console.AddCommand(new CommandGetSet<float>("light_lightStrength", &worldModel->lightData.lightStrength));
        console.AddCommand(new CommandGetSet<float>("light_ambientStrength", &worldModel->lightData.ambientStrength));

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

        Logger::SetCallOnLog(
                [&](std::string text)
                {
                    if(text.back() == '\n')
                        text.pop_back();

                    console.AddText(text);
                });

        Input::Update();

        guiManager.AddContainer(&console);

        //TODO: bind transformBuffer first to make sure default input layout generation works

        Timer timeSinceStart;
        timeSinceStart.Start();

        double frameTime = 0.0;
        unsigned long frameCount = 0;
        timer.ResetDelta();

        while(window.PollEvents())
        {
            if(window.IsPaused())
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
                timer.ResetDelta();
                continue;
            }

            timer.UpdateDelta();

            frameTime += timer.GetDeltaMillisecondsFraction();
            ++frameCount;

            if(frameTime > 3000)
            {
                Logger::LogLine(LOG_TYPE::INFO_NOWRITE, 1.0f / (frameCount / 3.0f) * 1000.0f);

                frameCount = 0;
                frameTime = 0.0;
            }

            if(!console.GetActive())
            {
                if(keysDown.count(KEY_CODE::A))
                    camera.MoveRight(-0.01f * timer.GetDeltaMillisecondsFraction());
                else if(keysDown.count(KEY_CODE::D))
                    camera.MoveRight(0.01f * timer.GetDeltaMillisecondsFraction());

                if(keysDown.count(KEY_CODE::W))
                    camera.MoveFoward(0.01f * timer.GetDeltaMillisecondsFraction());
                else if(keysDown.count(KEY_CODE::S))
                    camera.MoveFoward(-0.01f * timer.GetDeltaMillisecondsFraction());

                if(keysDown.count(KEY_CODE::V))
                    camera.MoveUp(0.01f * timer.GetDeltaMillisecondsFraction());
                else if(keysDown.count(KEY_CODE::C))
                    camera.MoveUp(-0.01f * timer.GetDeltaMillisecondsFraction());

                glm::vec2 mouseDelta = Input::GetMouseDelta();

                camera.Rotate(mouseDelta * 0.0025f);
            }

            guiManager.Update(timer.GetDelta());

            contentManager.HotReload();

            glClearColor(0.2f, 0.2f, 0.5f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            glEnable(GL_CULL_FACE);
            glEnable(GL_DEPTH_TEST);
            glCullFace(GL_BACK);

            worldModel->drawBinds["viewProjectionMatrix"] = camera.GetProjectionMatrix() * camera.GetViewMatrix();

            if(wireframe)
                glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

            worldModel->Draw(camera.GetPosition());

            if(wireframe)
                glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

            //glDepthFunc(GL_GREATER);

            glDisable(GL_DEPTH_TEST);
            glDisable(GL_CULL_FACE);

            spriteRenderer.Begin();

            guiManager.Draw(&spriteRenderer);

            spriteRenderer.End();

            window.SwapBuffers();

            Input::Update();
        }
    }
    else
        Logger::LogLine(LOG_TYPE::FATAL, "Couldn't create window");

    //IF_WINDOWS(Logger::Deinit());

    return 0;
}
