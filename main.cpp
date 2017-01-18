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

//Cool shit!
#ifdef _WIN32
#define IF_WINDOWS(...) __VA_ARGS__
#else
#define IF_WINDOWS(...)
#endif

#ifdef _WIN32

#include <windows.h>

int WINAPI WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int nShowCmd)
{
#else
int main(int argc, char* argv[])
{
#endif

    IF_WINDOWS(Logger::Init());

    OSWindow window;
    if(window.Create(1280, 720 IF_WINDOWS(, hInstance, nShowCmd)) == OSWindow::NONE)
    {
        IF_WINDOWS(_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF));

        glewExperimental = true;
        if(glewInit() != GLEW_OK)
            return 1;

        Timer timer;
        timer.UpdateDelta();

        Input::Init(&window, true);

        PerspectiveCamera camera;
        camera.InitFovVertical({0.0f, 0.0f, -5.0f}
                               , {0.0f, 0.0f, 0.0f}
                               , glm::half_pi<float>()
                               , 1280
                               , 720
                               , 0.01f
                               , 100.0f);

        std::set<KEY_CODE> keysDown;

        GUIManager guiManager;

        Input::RegisterKeyCallback(
                [&](const KeyState& keyState)
                {
                    if(keyState.action == KEY_ACTION::DOWN)
                    {
                        if(!keysDown.count(keyState.key))
                            keysDown.insert(keyState.key);
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

        window.RegisterFocusGainCallback(
                [&]()
                {
                    //Input::LockCursor(1280 / 2, 720 / 2);
                });

        window.RegisterFocusLossCallback(
                [&]()
                {
                    //Input::LockCursor(-1, -1);
                });

        Input::Update();

        //Input::LockCursor(1280 / 2, 720 / 2);

        ContentManager contentManager("content");

        CharacterSet* characterSet = contentManager.Load<CharacterSet>("UbuntuMono-R24.ttf");

        Console console;
        console.Init(&contentManager, Rect(0.0f, 0.0f, 1280.0f, 360.0f), console.GenerateDoomStyle(&contentManager, characterSet), console.GenerateDoomStyleBackgroundStyle(&contentManager), false, false, false, false);

        console.Activate();

        guiManager.AddContainer(&console);

//        GLVertexBuffer vertexBuffer;
//        vertexBuffer.Init<float, glm::vec3, glm::vec2>(GLEnums::BUFFER_USAGE::STATIC,
//                {
//                        -0.5f , -0.5f, 0.0f, 0.0f, 0.0f
//                        , 0.5f, -0.5f, 0.0f, 1.0f, 0.0f
//                        , -0.5f, 0.5f, 0.0f, 0.0f, 1.0f
//                        , 0.5f, 0.5f, 0.0f, 1.0f, 1.0f
//                });
//
//        std::vector<glm::mat4x4> transforms =
//                {
//                        glm::mat4x4()
//                        , glm::mat4x4()
//                        , glm::mat4x4()
//                        , glm::mat4x4()
//                        , glm::mat4x4()
//                };
//
//        for(int i = 0, end = transforms.size(); i < end; ++i)
//            transforms[i] = glm::translate(transforms[i], glm::vec3((float)i * 2.0f, 0.0f, 0.0f));
//
//        GLVertexBuffer transformBuffer;
//        transformBuffer.Init<glm::mat4x4, glm::mat4x4>(GLEnums::BUFFER_USAGE::STREAM, transforms);
//
//        GLIndexBuffer indexBuffer;
//        indexBuffer.Init(GLEnums::BUFFER_USAGE::STATIC, { 0, 2, 1, 3, 1, 2 });
//
//        GLDrawBinds binds;
//        binds.AddShaders(contentManager
//                         , GLEnums::SHADER_TYPE::VERTEX, "vertex.glsl"
//                         , GLEnums::SHADER_TYPE::PIXEL, "pixel.glsl");
//
//        GLInputLayout vertexBufferLayout;
//        vertexBufferLayout.SetInputLayout<glm::vec3, glm::vec2>();
//
//        GLInputLayout transformBufferLayout;
//        transformBufferLayout.SetInputLayout<glm::mat4x4>(2);
//
//        //TODO: bind transformBuffer first to make sure default input layout generation works
//        binds.AddBuffers(&vertexBuffer, vertexBufferLayout
//                         , &transformBuffer, transformBufferLayout
//                         , &indexBuffer);
//        binds.AddUniform("color", 0.0f);
//        binds.AddUniform("viewProjectionMatrix", glm::mat4x4());
//
//        if(!binds.Init())
//            return 2;

        Timer timeSinceStart;
        timeSinceStart.Start();

        Texture* testTexture = contentManager.Load<Texture>("testTexture.png");

        SpriteRenderer spriteRenderer;
        if(!spriteRenderer.Init(contentManager, 1280, 720))
        {
            Logger::LogLine(LOG_TYPE::FATAL, "Couldn't initialize sprite renderer");
            return 1;
        }

        while(window.PollEvents())
        {
            timer.UpdateDelta();

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
            //if(mouseDelta.x != mouseDelta.y != 0.0f)
            //    std::cout << mouseDelta.x << ", " << mouseDelta.y << std::endl;
            camera.Rotate(mouseDelta * 0.0025f);

            guiManager.Update(timer.GetDelta());

            contentManager.HotReload();

            glClearColor(0.2f, 0.2f, 0.5f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT);

            //binds["color"] = std::fmod(timeSinceStart.GetTimeMillisecondsFraction() / 1000.0f, 1.0f);
            //binds["viewProjectionMatrix"] = camera.GetProjectionMatrix() * camera.GetViewMatrix();

            //binds.Bind();
            //glBindTexture(GL_TEXTURE_2D, texture->GetTexture());

            //binds.DrawElementsInstanced(5);

            //glBindTexture(GL_TEXTURE_2D, 0);
            //binds.Unbind();

            spriteRenderer.Begin();
            //spriteRenderer.Draw(Rect(64.0f, 64.0f, 64.0f, 64.0f));

            guiManager.Draw(&spriteRenderer);

            spriteRenderer.End();

            window.SwapBuffers();

            Input::Update();
        }
    }
    else
        Logger::LogLine(LOG_TYPE::FATAL, "Couldn't create window");

    IF_WINDOWS(Logger::Deinit());

    return 0;
}
