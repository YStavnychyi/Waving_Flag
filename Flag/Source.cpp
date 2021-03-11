#include <iostream>
#include <glut.h>
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
using namespace std;

HGLRC           hRC = NULL;                           // Permanent Rendering Context
HDC             hDC = NULL;                           // Private GDI Device Context
HWND            hWnd = NULL;                          // Holds Our Window Handle
HINSTANCE       hInstance;                          // Holds The Instance Of The Application

bool    keys[256];                              // Array Used For The Keyboard Routine
bool    active = TRUE;                            // Window Active Flag Set To TRUE By Default
bool    fullscreen = TRUE;                        // Fullscreen Flag Set To Fullscreen Mode By Default

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

GLvoid ReSizeScene(GLsizei width, GLsizei height) {  // Resize And Initialize The GL Window
    if (height == 0) {
        height = 1;
    }
    glViewport(0, 0, width, height);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45.0f, (GLfloat)width / (GLfloat)height, 0.1f, 100.0f);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

int InitGL(GLvoid) {                                 // All Setup For OpenGL Goes Here
    glShadeModel(GL_SMOOTH);
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClearDepth(1.0f);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
    return true;
}

int DrawGLScene(GLvoid) {                            // Here's Where We Do All The Drawing
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();
    return true;
}

GLvoid KillGLWindow(GLvoid) {                        // Properly Kill The Window
    if (fullscreen) {
        ChangeDisplaySettings(NULL, 0);
        ShowCursor(true);
    }
    if (hRC) {
        if (!wglMakeCurrent(NULL, NULL)) {
            MessageBox(NULL, L"Release Of DC And RC Failed.", L"SHUTDOWN ERROR", MB_OK | MB_ICONINFORMATION);
        }
        if (!wglDeleteContext(hRC)) {
            MessageBox(NULL, L"Release Rendering Context Failed.", L"SHUTDOWN ERROR", MB_OK | MB_ICONINFORMATION);
        }
        hRC = NULL;
    }
    if (hDC && !ReleaseDC(hWnd, hDC)) {
        MessageBox(NULL, L"Release Device Context Failed.", L"SHUTDOWN ERROR", MB_OK | MB_ICONINFORMATION);
        hDC = NULL;
    }
    if (hWnd && !DestroyWindow(hWnd)) {
        MessageBox(NULL, L"Could Not Release hWnd.", L"SHUTDOWN ERROR", MB_OK | MB_ICONINFORMATION);
        hWnd = NULL;
    }
    if (!UnregisterClass(L"OpendGL", hInstance)) {
        MessageBox(NULL, L"Could Not Unregister Class.", L"SHUTDOWN ERROR", MB_OK | MB_ICONINFORMATION);
        hInstance = NULL;
    }
}

bool CreateGLWindow(char* title, int width, int height, int bits, bool fullscreenflag) {
    GLint PixelFormat;
    WNDCLASS wc;
    DWORD dwExStyle;
    DWORD dwStyle;

    RECT WindowRect;
    WindowRect.left = (long)0;
    WindowRect.right = (long)width;
    WindowRect.top = (long)0;
    WindowRect.bottom = (long)height;

    fullscreen = fullscreenflag;

    hInstance = GetModuleHandle(NULL);
    wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
    wc.lpfnWndProc = (WNDPROC)WndProc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = hInstance;
    wc.hIcon = LoadIcon(NULL, IDI_WINLOGO);
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = NULL;
    wc.lpszMenuName = NULL;
    wc.lpszClassName = L"OpenGL";

    if (!RegisterClass(&wc)) {
        MessageBox(NULL, L"Failed To Register The Window Class.", L"ERROR", MB_OK | MB_ICONEXCLAMATION);
        return false;
    }
    if (fullscreen) {
        DEVMODE dmScreenSettings;
        memset(&dmScreenSettings, 0, sizeof(dmScreenSettings));
        dmScreenSettings.dmSize = sizeof(dmScreenSettings);
        dmScreenSettings.dmPelsWidth = width;
        dmScreenSettings.dmPelsHeight = height;
        dmScreenSettings.dmBitsPerPel = bits;
        dmScreenSettings.dmFields = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;
        if (ChangeDisplaySettings(&dmScreenSettings, CDS_FULLSCREEN) != DISP_CHANGE_SUCCESSFUL) {
            if (MessageBox(NULL, L"The Requested Fullscreen Mode Is Not Supported By\nYour Video Card. Use Windowed Mode Instead?", L"GL", MB_YESNO | MB_ICONEXCLAMATION) == IDYES) {
                fullscreen = false;
            }
            else {
                MessageBox(NULL, L"Program Will Now Close.", L"ERROR", MB_OK | MB_ICONSTOP);
                return FALSE;
            }
        }
    }
    if (fullscreen) {
        dwExStyle = WS_EX_APPWINDOW;
        dwStyle = WS_POPUP;
        ShowCursor(FALSE);
    }
    else {
        dwExStyle = WS_EX_APPWINDOW | WS_EX_WINDOWEDGE;
        dwStyle = WS_OVERLAPPEDWINDOW;
    }
    AdjustWindowRectEx(&WindowRect, dwStyle, FALSE, dwExStyle);
    if (!(hWnd = CreateWindowEx(dwExStyle, L"OpenGL", L"Flag", WS_CLIPSIBLINGS | WS_CLIPCHILDREN | dwStyle, 0, 0, WindowRect.right - WindowRect.left, WindowRect.bottom - WindowRect.top, NULL, NULL, hInstance, NULL))) {
        KillGLWindow();
        MessageBox(NULL, L"Window Creation Error.", L"ERROR", MB_OK | MB_ICONEXCLAMATION);
        return false;
    }

    static  PIXELFORMATDESCRIPTOR pfd =                  // pfd Tells Windows How We Want Things To Be
    {
        sizeof(PIXELFORMATDESCRIPTOR),
        1,
        PFD_DRAW_TO_WINDOW |
        PFD_SUPPORT_OPENGL |
        PFD_DOUBLEBUFFER,
        PFD_TYPE_RGBA,
        16,
        0, 0, 0, 0, 0, 0,
        0,
        0,
        0,
        0, 0, 0, 0,
        16,
        0,
        0,
        PFD_MAIN_PLANE,
        0,
        0, 0, 0
    };

    if (!(hDC = GetDC(hWnd))) {
        KillGLWindow();
        MessageBox(NULL, L"Can't Create A GL Device Context.", L"ERROR", MB_OK | MB_ICONEXCLAMATION);
        return false;
    }

    if (!(PixelFormat = ChoosePixelFormat(hDC, &pfd))) {
        KillGLWindow();
        MessageBox(NULL, L"Can't Find A Suitable PixelFormat.", L"ERROR", MB_OK | MB_ICONEXCLAMATION);
        return false;
    }

    if (!SetPixelFormat(hDC, PixelFormat, &pfd)) {
        KillGLWindow();
        MessageBox(NULL, L"Can't Set The PixelFormat.", L"ERROR", MB_OK | MB_ICONEXCLAMATION);
        return false;
    }

    if (!(hRC = wglCreateContext(hDC))) {
        KillGLWindow();
        MessageBox(NULL, L"Can't Create A GL Rendering Context.", L"ERROR", MB_OK | MB_ICONEXCLAMATION);
        return FALSE;
    }

    if (!wglMakeCurrent(hDC, hRC)) {
        KillGLWindow();
        MessageBox(NULL, L"Can't Activate The GL Rendering Context.", L"ERROR", MB_OK | MB_ICONEXCLAMATION);
        return FALSE;
    }

    ShowWindow(hWnd, SW_SHOW);
    SetForegroundWindow(hWnd);
    SetFocus(hWnd);
    ReSizeScene(width, height);

    if (!InitGL()) {
        KillGLWindow();
        MessageBox(NULL, L"Initialization Failed.", L"ERROR", MB_OK | MB_ICONEXCLAMATION);
        return false;
    }

    return true;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{

    return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

int Winmain()
{
    cout << "Hello world!" << endl;
    return 0;
}
