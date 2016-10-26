#include <chrono>
#include <thread>
#include <set>

#include "window.h"
#include "timer.h"
#include "logger.h"
#include "input.h"
#include "perspectiveCamera.h"
#include "vertexBuffer.h"
#include "glVertexShader.h"
#include "glPixelShader.h"
#include "glShaderProgram.h"

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
                                   });
        Input::RegisterMouseButtonCallback(
                [&](const MouseButtonState& buttonState)
                {

                });

        window.RegisterFocusGainCallback(
                [&]()
                                         {
                                             Input::LockCursor(1280 / 2, 720 / 2);
                                         });

        window.RegisterFocusLossCallback(
                [&]()
                                         {
                                             Input::LockCursor(-1, -1);
                                         });

        Input::Update();

        Input::LockCursor(1280 / 2, 720 / 2);

        GLVertexShader vertexShader;
        vertexShader.Load("vertex.glsl");

        GLPixelShader pixelShader;
        pixelShader.Load("pixel.glsl");

        GLfloat vertices[] = {
                0.0f,  0.5f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f
                , 0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f
                , -0.5f, -0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f
        };

        VertexBuffer vertexBuffer;
        vertexBuffer.Init<glm::vec2, glm::vec3, glm::vec3>(GLEnums::BUFFER_USAGE::STATIC, vertices, 3, false);
        vertexBuffer.Bind();

        GLShaderProgram shaderProgram;
        shaderProgram.Load(&vertexShader, &pixelShader
                           , 2, GLEnums::DATA_TYPE::FLOAT, false
                           , 3, GLEnums::DATA_TYPE::FLOAT, false
                           , 3, GLEnums::DATA_TYPE::FLOAT, false); //TODO: Pad syntax

        vertexBuffer.Unbind();

        shaderProgram.Bind();

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
            camera.Rotate(mouseDelta * 0.001f);

            glClearColor(0.2f, 0.2f, 0.5f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT);

            glDrawArrays(GL_TRIANGLES, 0, 3);

//            shaderProgram.Unbind();
//            vertexBuffer.Unbind();

            window.SwapBuffers();

            Input::Update();

            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
    }

    IF_WINDOWS(Logger::Deinit());

    return 0;
}
