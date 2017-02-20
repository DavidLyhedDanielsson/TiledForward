#ifndef window_h__
#define window_h__

#ifdef _WIN32
#include <windows.h>
#else
#include <GL/gl3w.h>
#include <GL/glx.h>
#endif

#ifdef USE_DX
#include <d3d11.h>
#else
#include <GL/gl3w.h>
#include <GL/gl.h>
#endif

#include <memory>
#include <vector>
#include <functional>

struct GraphicsSettings;

class OSWindow
{
	friend class ScreenManager;
public:
	OSWindow();
	~OSWindow();

	enum CREATE_WINDOW_ERROR
	{
        NONE
        , COULDNT_CREATE_DISPLAY
        , COULDNT_GET_CONFIG
        , COULDNT_CREATE_WINDOW
        , COULDNT_CHOOSE_PIXEL_FORMAT
		, COULDNT_SET_PIXEL_FORMAT
        , WELL_SHIT
	};

    CREATE_WINDOW_ERROR Create(unsigned int width, unsigned int height
#ifdef _WIN32
            , HINSTANCE hInstance, int nCmdShow
#endif
    );

    //TODO: GL error callback on Windows

#ifdef _WIN32
    HWND hWnd;
    HINSTANCE hInstance;

#ifdef USE_DX
#else
	HGLRC hglrc;
	HDC hdc;

	HGLRC GetHGLRC();
	HDC GetHDC();
#endif

	LRESULT WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

	HWND GetHWND();

#ifdef USE_DX
    CREATE_WINDOW_ERROR CreateDirectXWindow();
    //GLXContext CreateNewSharedContext();
#else // USE_DX
    OSWindow::CREATE_WINDOW_ERROR CreateOpenGLWindow(int nCmdShow);
#endif // USE_DX

#else // _WIN32
    CREATE_WINDOW_ERROR CreateXWindow();

	Display* GetDisplay();
	Window GetWindow();
	GLXContext GetContext();
	Colormap GetColormap();
#endif // _WIN32

    bool PollEvents();

	bool IsPaused() const;

	int GetResolutionX() const;
	int GetResolutionY() const;

    void SwapBuffers();

    void RegisterFocusLossCallback(std::function<void()> callback);
	void RegisterFocusGainCallback(std::function<void()> callback);
	void RegisterWindowSizeChangeCallback(std::function<void(int, int)> callback);

protected:
	unsigned int width;
	unsigned int height;

    unsigned int resizedWidth;
    unsigned int resizedHeight;

private:
	bool paused;

    std::function<void()> focusLossCallback;
	std::function<void()> focusGainCallback;
	std::function<void(int, int)> windowSizeChangeCallback;

#ifdef _WIN32
	bool CreateWindowsWindow(int nCmdShow);
#else // _WIN32
    Display* display;
    Window window;
    GLXContext context;
    Colormap colormap;

    Atom wmDeleteWindow;

    GLXFBConfig GetConfig();

    void CreateXWindow(Window& window, Colormap& colormap, GLXFBConfig fbConfig);

    GLXContext CreateContext(GLXFBConfig fbConfig);
#endif // _WIN32

	GraphicsSettings* graphicsSettings;

    void CleanUp();
};

#ifdef GL_ERROR_CALLBACK
void GLMessageCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar *message
                       , const void *userParam);
#endif

#ifdef _WIN32
static OSWindow* window = nullptr;

LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
#endif // _WIN32
#endif // window_h__