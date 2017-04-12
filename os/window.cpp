#ifdef _WIN32
#include <windows.h>
#else
#include <X11/Xlib.h>
#include <X11/XKBlib.h>
#endif
#include "window.h"
#include "input.h"

#ifdef USE_LOGGER
#include "../logger.h"

#define LOG_DEBUG(message) Logger::LogLine(LOG_TYPE::DEBUG, message);
#define LOG_INFO(message) Logger::LogLine(LOG_TYPE::INFO, message);
#define LOG_ERROR(message) Logger::LogLine(LOG_TYPE::ERROR, message);
#define LOG_WARNING(message) Logger::LogLine(LOG_TYPE::WARNING, message);
#else // USE_LOGGER
#include <iostream>

#ifndef NDEBUG
#define LOG_DEBUG(message) std::cout << message << std::endl;
#else // NDEBUG
#define LOG_DEBUG(message)
#endif
#define LOG_INFO(message) std::cout << message << std::endl;
#define LOG_ERROR(message) std::cout << message << std::endl;
#define LOG_WARNING(message) std::cout << message << std::endl;
#endif // USE_LOGGER

OSWindow::OSWindow()
	: width(-1)
	, height(-1)
	, paused(false)
#ifndef _WIN32
    , display(nullptr)
#endif
{
#ifdef _WIN32
	window = this;
#else
    XInitThreads();
#endif // _WIN32
}

OSWindow::~OSWindow()
{
    CleanUp();
}

OSWindow::CREATE_WINDOW_ERROR OSWindow::Create(unsigned int width, unsigned int height
#ifdef _WIN32
                                               , HINSTANCE hInstance, int nCmdShow
#endif
)
{
    this->width = width;
    this->height = height;

    CREATE_WINDOW_ERROR errorCode = NONE;

#ifdef _WIN32
    //this->hWnd = hWnd;
    this->hInstance = hInstance;
#ifdef USE_DX
#else
    errorCode = CreateOpenGLWindow(nCmdShow);
#endif // USE_DX
#else // _WIN32
    errorCode = CreateXWindow();
#endif // _WIN32

#ifdef GL_ERROR_CALLBACK
    glEnable(GL_DEBUG_OUTPUT);
    glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);

    if(glDebugMessageCallback)
    {
        glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, 0, GL_TRUE);

        glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
        glDebugMessageCallback(GLMessageCallback, nullptr);
    }
    else
        LOG_DEBUG("GL_ERROR_CALLBACK was defined but glDebugMessageCallback is false");
#endif

    return errorCode;
}

#ifdef _WIN32
struct MonitorData
{
	int targetMonitor;
	int monitor;
	RECT dimensions;
};

BOOL CALLBACK MonitorEnumProc(
	_In_ HMONITOR hMonitor,
	_In_ HDC      hdcMonitor,
	_In_ LPRECT   lprcMonitor,
	_In_ LPARAM   dwData
	)
{
	MonitorData* data = reinterpret_cast<MonitorData*>(dwData);

	data->dimensions = *lprcMonitor;
	if(data->monitor == data->targetMonitor)
		return FALSE;

	data->monitor++;
	return TRUE;
}

bool OSWindow::CreateWindowsWindow(int nCmdShow)
{
    MonitorData monitorData;
	ZeroMemory(&monitorData, sizeof(monitorData));
	monitorData.targetMonitor = 0;

	EnumDisplayMonitors(NULL, NULL, MonitorEnumProc, reinterpret_cast<LPARAM>(&monitorData));

	WNDCLASS wndClass;

	wndClass.cbClsExtra = 0;
	wndClass.cbWndExtra = 0;
	wndClass.hbrBackground = static_cast<HBRUSH>(GetStockObject(WHITE_BRUSH));
	wndClass.hCursor = LoadCursor(0, IDC_ARROW);
	wndClass.hIcon = LoadIcon(0, IDI_APPLICATION);
	wndClass.hInstance = hInstance;
	wndClass.lpfnWndProc = ::WndProc;
	wndClass.lpszClassName = "DX11 OSWindow";
	wndClass.lpszMenuName = 0;
	wndClass.style = CS_HREDRAW | CS_VREDRAW;

	if(!RegisterClass(&wndClass))
		return false;

	RECT clientRect;
	clientRect.left = monitorData.dimensions.left;
	clientRect.right = monitorData.dimensions.left + 1280;
	clientRect.top = monitorData.dimensions.top;
	clientRect.bottom = monitorData.dimensions.top + 720;

	DWORD windowStyle = WS_OVERLAPPEDWINDOW;

	AdjustWindowRect(&clientRect, windowStyle, false);

	hWnd = CreateWindow("DX11 OSWindow"
						, "DX11 OSWindow"
						, windowStyle
						, monitorData.dimensions.left
						, monitorData.dimensions.top
						, clientRect.right - clientRect.left
						, clientRect.bottom - clientRect.top
						, 0
						, 0
						, hInstance
						, 0);

	if(hWnd == nullptr)
		return false;

	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);

	return true;
}

HGLRC OSWindow::GetHGLRC()
{
	return hglrc;
}

HDC OSWindow::GetHDC()
{
	return hdc;
}

LRESULT OSWindow::WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch(msg)
	{
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	case WM_LBUTTONDOWN:
	case WM_MBUTTONDOWN:
	case WM_RBUTTONDOWN:
	case WM_LBUTTONUP:
	case WM_MBUTTONUP:
	case WM_RBUTTONUP:
		//Input::MouseButtonEvent(msg, wParam);
		break;
	case WM_KEYDOWN:
		Input::KeyEvent(msg, wParam, lParam);
		break;
	case WM_KEYUP:
		Input::KeyEvent(msg, wParam, lParam);
		break;
	case WM_MOUSEWHEEL:
		Input::ScrollEvent(GET_WHEEL_DELTA_WPARAM(wParam));
		break;
	case WM_CHAR:
		Input::CharEvent(static_cast<int>(wParam));
		break;
	case WM_ACTIVATE:
		if(LOWORD(wParam) == WA_INACTIVE)
		{
			paused = true;

			if(focusLossCallback != nullptr)
			    focusLossCallback();
		}
		else
        {
			paused = false;

			if(focusGainCallback != nullptr)
			    focusGainCallback();
		}
		break;
	case WM_EXITSIZEMOVE:
	{
		RECT clientRect;
		GetClientRect(hWnd, &clientRect);

		if(clientRect.right - clientRect.left != width
			|| clientRect.bottom - clientRect.top != height)
		{
			//Resize(clientRect);
		}

		return DefWindowProc(hWnd, msg, wParam, lParam);
		break;
	}
	default:
		return DefWindowProc(hWnd, msg, wParam, lParam);
		break;
	}

	//Shouldn't ever happen
	return 0;
}

HWND OSWindow::GetHWND()
{
	return hWnd;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	return window->WndProc(hWnd, msg, wParam, lParam);
}

#ifdef USE_DX
OSWindow::CREATE_WINDOW_ERROR OSWindow::CreateDirectXWindow()
{
    return WELL_SHIT;
}
#else // USE_DX
OSWindow::CREATE_WINDOW_ERROR OSWindow::CreateOpenGLWindow(int nCmdShow)
{
    CREATE_WINDOW_ERROR errorCode = NONE;

    if(!CreateWindowsWindow(nCmdShow))
    {
        CleanUp();
        return COULDNT_CREATE_WINDOW;
    }

    PIXELFORMATDESCRIPTOR pixelFormatDescriptor;
    pixelFormatDescriptor.nSize = sizeof(PIXELFORMATDESCRIPTOR);
    pixelFormatDescriptor.nVersion = 1;
    pixelFormatDescriptor.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
    pixelFormatDescriptor.dwLayerMask = PFD_MAIN_PLANE;
    pixelFormatDescriptor.iPixelType = PFD_TYPE_COLORINDEX;
    pixelFormatDescriptor.cColorBits = 8;
    pixelFormatDescriptor.cDepthBits = 16;
    pixelFormatDescriptor.cAccumBits = 0;
    pixelFormatDescriptor.cStencilBits = 0;

    hdc = GetDC(hWnd);

    int pixelFormat = ChoosePixelFormat(hdc, &pixelFormatDescriptor);

    if(pixelFormat == 0)
    {
        CleanUp();
        return COULDNT_CHOOSE_PIXEL_FORMAT;
    }

    if(!SetPixelFormat(hdc, pixelFormat, &pixelFormatDescriptor))
    {
        CleanUp();
        return COULDNT_SET_PIXEL_FORMAT;
    }

    hglrc = wglCreateContext(hdc);
    wglMakeCurrent(hdc, hglrc);
	
	if (glewInit() != GLEW_OK)
		return WELL_SHIT;

	std::string version = (char*)glGetString(GL_VERSION);

    return NONE;
}
#endif // USE_DX
#else // _WIN32
typedef GLXContext (*glXCreateContextAttribsARBProc)(Display*, GLXFBConfig, GLXContext, Bool, const int*);

static bool ctxErrorOccurred = false;
static int ctxErrorHandler(Display* display, XErrorEvent* event)
{
    ctxErrorOccurred = true;

    char buffer[2048];
    XGetErrorText(display, event->error_code, buffer, 2048);

    Logger::LogLine(LOG_TYPE::FATAL, std::string(buffer));

    return 0;
}

OSWindow::CREATE_WINDOW_ERROR OSWindow::CreateXWindow()
{
    display = XOpenDisplay(NULL);
    if(display == nullptr)
    {
        CleanUp();
        return COULDNT_CREATE_DISPLAY;
    }

    GLXFBConfig fbConfig = GetConfig();
    if(fbConfig == nullptr)
    {
        CleanUp();
        return COULDNT_GET_CONFIG;
    }

    CreateXWindow(window, colormap, fbConfig);
    if(!window)
    {
        CleanUp();
        return COULDNT_CREATE_WINDOW;
    }

    XMapWindow(display, window);
    wmDeleteWindow = XInternAtom(display, "WM_DELETE_WINDOW", False);
    XSetWMProtocols(display, window, &wmDeleteWindow, 1);

    XStoreName(display, window, "OpenGL Window");

    context = CreateContext(fbConfig);
    if(ctxErrorOccurred || context == nullptr)
    {
        CleanUp();
        return WELL_SHIT;
    }

    glXMakeCurrent(display, window, context);

    if(gl3wInit() != 0)
    {
        CleanUp();
        return WELL_SHIT;
    }

    return NONE;
}

GLXFBConfig OSWindow::GetConfig()
{
    int attributes[] =
            {
                    GLX_X_RENDERABLE, True
                    , GLX_DRAWABLE_TYPE, GLX_WINDOW_BIT
                    , GLX_RENDER_TYPE, GLX_RGBA_BIT
                    , GLX_X_VISUAL_TYPE, GLX_TRUE_COLOR
                    , GLX_RED_SIZE, 8
                    , GLX_GREEN_SIZE, 8
                    , GLX_BLUE_SIZE, 8
                    , GLX_ALPHA_SIZE, 8
                    , GLX_DEPTH_SIZE, 24
                    , GLX_STENCIL_SIZE, 8
                    , GLX_DOUBLEBUFFER, True
                    //, GLX_SAMPLE_BUFFERS, 1
                    //, GLX_SAMPLES, 8
                    , None
            };

    int fbConfigSize = -1;
    GLXFBConfig* fbConfigs = glXChooseFBConfig(display, DefaultScreen(display), attributes, &fbConfigSize);
    GLXFBConfig returnConfig = fbConfigSize > 0 ? fbConfigs[0] : nullptr;

    XFree(fbConfigs);

    return returnConfig;
}

void OSWindow::CreateXWindow(Window& window, Colormap& colormap, GLXFBConfig fbConfig)
{
    XVisualInfo* visualInfo = glXGetVisualFromFBConfig(display, fbConfig);

    XSetWindowAttributes windowAttributes;
    colormap = XCreateColormap(display, RootWindow(display, visualInfo->screen), visualInfo->visual, AllocNone);
    windowAttributes.border_pixel = 0;
    windowAttributes.event_mask = ExposureMask | StructureNotifyMask | KeyPressMask | KeyReleaseMask | ButtonPressMask | ButtonReleaseMask | PointerMotionMask | FocusChangeMask;
    windowAttributes.colormap = colormap;
    windowAttributes.background_pixmap = None;

    window = XCreateWindow(display
                           , RootWindow(display, visualInfo->screen)
                           , 0
                           , 0
                           , width
                           , height
                           , 0
                           , visualInfo->depth
                           , InputOutput
                           , visualInfo->visual
                           , CWBorderPixel | CWColormap | CWEventMask
                           , &windowAttributes);

    XFree(visualInfo);
}

GLXContext OSWindow::CreateContext(GLXFBConfig fbConfig)
{
    int (* oldHandler)(Display*, XErrorEvent*) = XSetErrorHandler(&ctxErrorHandler);

    int contextAttributes[] =
            {
                    GLX_CONTEXT_MAJOR_VERSION_ARB, 4
                    , GLX_CONTEXT_MINOR_VERSION_ARB, 5
#ifndef NDEBUG
                    , GLX_CONTEXT_FLAGS_ARB, GLX_CONTEXT_DEBUG_BIT_ARB
#endif // NDEBUG
                    , None
            };

    glXCreateContextAttribsARBProc glXCreateContextAttribsARB = 0;
    glXCreateContextAttribsARB = (glXCreateContextAttribsARBProc)glXGetProcAddressARB(
            (const GLubyte*)"glXCreateContextAttribsARB");

    GLXContext context = glXCreateContextAttribsARB(display, fbConfig, nullptr, True, contextAttributes);
    XSync(display, False);

    if(ctxErrorOccurred)
        context = nullptr;

    XSetErrorHandler(oldHandler);

    return context;
}

Display* OSWindow::GetDisplay()
{
    return display;
}

Window OSWindow::GetWindow()
{
    return window;
}

GLXContext OSWindow::GetContext()
{
    return context;
}

Colormap OSWindow::GetColormap()
{
    return colormap;
}
#endif // _WIN32

bool OSWindow::IsPaused() const
{
	return paused;
}

int OSWindow::GetResolutionX() const
{
	return static_cast<int>(width);
}

int OSWindow::GetResolutionY() const
{
	return static_cast<int>(height);
}

void OSWindow::SwapBuffers()
{
#ifdef USE_DX
#else
#ifdef _WIN32
	::SwapBuffers(hdc);
#else
    glXSwapBuffers(display, window);
#endif // _WIN32
#endif // USE_DX
}

void OSWindow::RegisterFocusLossCallback(std::function<void()> callback)
{
    focusLossCallback = callback;
}

void OSWindow::RegisterFocusGainCallback(std::function<void()> callback)
{
    focusGainCallback = callback;
}

void OSWindow::RegisterWindowSizeChangeCallback(std::function<void(int, int)> callback)
{
    windowSizeChangeCallback = callback;
}

void OSWindow::CleanUp()
{
#ifdef USE_DX

#else
#ifdef _WIN32
#else
    if(display == nullptr)
        return;

    glXMakeCurrent(display, 0, 0);

    if(context != nullptr)
        glXDestroyContext(display, context);

    if(window)
        XDestroyWindow(display, window);
    if(colormap)
        XFreeColormap(display, colormap);
    if(display != nullptr)
        XCloseDisplay(display);

    display = nullptr;
#endif // _WIN32
#endif // USE_DX
}

bool OSWindow::PollEvents()
{
#ifdef _WIN32
	MSG msg = { 0 };

	while(PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
	{
		TranslateMessage(&msg);

		if(msg.message == WM_QUIT)
		{
			CleanUp();
			return false;
		}

		DispatchMessage(&msg);
	}
#else // _WIN32
    while(XPending(display) > 0)
    {
        XEvent event;
        XNextEvent(display, &event);

        switch(event.type)
        {
            case KeyPress:
            {
                char text[255];
                KeySym key;
                int value = XLookupString(&event.xkey, text, 255, &key, 0);
                for(int i = 0; i < value; ++i)
                    Input::CharEvent(text[i]);

                Input::KeyEvent(event.xkey);
            }
                break;
            case KeyRelease:
                Input::KeyEvent(event.xkey);
                break;
            case ButtonPress:
            case ButtonRelease:
                Input::MouseButtonEvent(event.xbutton);
                break;
            case ClientMessage:
                if((Atom)event.xclient.data.l[0] == wmDeleteWindow)
                    return false;
                break;
            case FocusIn:
                if(focusGainCallback != nullptr)
                    focusGainCallback();

                paused = false;

                if(width != resizedWidth
                        || height != resizedHeight)
                {
                    if(windowSizeChangeCallback != nullptr)
                    {
                        windowSizeChangeCallback(resizedWidth, resizedHeight);

                        width = resizedWidth;
                        height = resizedHeight;
                    }
                }
                break;
            case FocusOut:
                if(focusLossCallback != nullptr)
                    focusLossCallback();

                paused = true;
                break;
            case ConfigureNotify:
                resizedWidth = (unsigned int)event.xconfigure.width;
                resizedHeight = (unsigned int)event.xconfigure.height;
                break;
            default:
                break;
        }
    }
#endif // _WIN32

    return true;
}

#ifdef GL_ERROR_CALLBACK
void GLMessageCallback(GLenum source
                       , GLenum type
                       , GLuint id
                       , GLenum severity
                       , GLsizei length
                       , const GLchar* message
                       , const void* userParam)
{
    if(id == 131185 || id == 131204 || id == 131184 || id == 131169 || id == 131186) // Workaround for Nvidias apparently buggy driver
        return;

    std::string stringMessage = "---------- OpenGL message ----------\n";

    stringMessage += "Severity: ";
    switch(severity)
    {
        case GL_DEBUG_SEVERITY_LOW:
            stringMessage += "Low";
            break;
        case GL_DEBUG_SEVERITY_MEDIUM:
            stringMessage += "Medium";
            break;
        case GL_DEBUG_SEVERITY_HIGH:
            stringMessage += "High";
            break;
        case GL_DEBUG_SEVERITY_NOTIFICATION:
            stringMessage += "Notification";
            break;
        default:
            stringMessage += "Unknown";
            break;
    }

    stringMessage += '\n';
    stringMessage += std::string(message) + "\n";

    stringMessage += "type: ";
    switch(type)
    {
        case GL_DEBUG_TYPE_ERROR:
            stringMessage += "Error";
            break;
        case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
            stringMessage += "Deprecated";
            break;
        case GL_DEBUG_TYPE_PORTABILITY:
            stringMessage += "Portability";
            break;
        case GL_DEBUG_TYPE_PERFORMANCE:
            stringMessage += "Performance";
            break;
        case GL_DEBUG_TYPE_OTHER:
            stringMessage += "Other";
            break;
        case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
            stringMessage += "Undefined behaviour";
            break;
        default:
            stringMessage += "UNKNOWN";
            break;
    }

    stringMessage += " (" + std::to_string(id) + ")";

    switch(severity)
    {
        case GL_DEBUG_SEVERITY_LOW:
        case GL_DEBUG_SEVERITY_MEDIUM:
        case GL_DEBUG_SEVERITY_HIGH:
            LOG_WARNING(stringMessage);
            break;
        default:
        case GL_DEBUG_SEVERITY_NOTIFICATION:
            LOG_DEBUG(stringMessage);
            break;
    }
}
#endif