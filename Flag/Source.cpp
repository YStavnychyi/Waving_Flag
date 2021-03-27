#define _USE_MATH_DEFINES
#define STB_IMAGE_IMPLEMENTATION
#define BITMAP_ID 0x4D42

#include <iostream>
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <glew.h>
#include <gl/GL.h>
#include <gl/GLU.h>
#include <stb_image.h>
using namespace std;

//Zmienne globalne
HDC g_HDC;								//globalny kontekst urzdządzenia
bool fullScreen = false;				//true = tryb pełnoekrankowy
                                        //false = tryb okienkowy
bool keyPressed[256];					//tablica przyciśnięć klawisz

//Opis tekstury
BITMAPINFOHEADER bitmapInfoHeader;		//naglówek pliku
unsigned char* bitmapData;				//dane tekstury
unsigned int texture = 0;					//obiekt tekstury
//Opis flagi
float flagPoints[36][20][3];			//flaga 36x20
float wrapValue;						//przenosi fali z końca na początek flagi

//Inicjacja punktów flagi za pomocą funkcji sin()
void InitializeFlag()
{
    int xIdx;								//licznik w plaszczyżnie x
    int yIdx;								//licznik w plaszczyżnie y
    float sinTemp;

    //Przegląda wszystkie punkty flagi w pętli i wyznacza wartość funkcji sin dla
    //współrzędnej z. Wszpółrzędne x i y otrzymują wartość odpowiedniego licznika pętli
    for (xIdx = 0; xIdx < 36; xIdx++)
    {
        for (yIdx = 0; yIdx < 20; yIdx++)
        {

            flagPoints[xIdx][yIdx][0] = (float)xIdx;
            flagPoints[xIdx][yIdx][1] = (float)yIdx;
            sinTemp = (((float)xIdx * 20.0f) / 360.0f) * 2.0f * M_PI;
            flagPoints[xIdx][yIdx][2] = (float)sin(sinTemp);
        }
    }
}

//Zainicjowania grafiki OpenGl
void Initialize()
{
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);	//tlo w czarnym kolorze
    glEnable(GL_DEPTH_TEST);				//usuwanie ukrytych powierzchni
    glDepthFunc(GL_LESS);
    glShadeModel(GL_SMOOTH);
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_CULL_FACE);					//brak obliczeń dla niewidocznych stron wielokątów
    glFrontFace(GL_CCW);					//niewidoczne strony posiadają porządek wierzcholków
                                            //przeciwny do kierunku ruchu wskazówek zegara
    glEnable(GL_TEXTURE_2D);				//wlącza tekstury 2D

    //ładuje obraz tekstury
    int width, height, nrChannels;
    bitmapData = stbi_load("flag.bmp", &width, &height, &nrChannels, STBI_rgb_alpha);

    glGenTextures(1, &texture);             //tworzy obiekt tekstury
    glBindTexture(GL_TEXTURE_2D, texture);  //aktywuje obiekt tekstury
    gluBuild2DMipmaps(GL_TEXTURE_2D, GL_RGBA, width, height, GL_RGBA, GL_UNSIGNED_BYTE, bitmapData);
    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

    InitializeFlag();
}

//Obraz flagi dla pojedynczej klatki animacjii
void DrawFlag()
{
    int xIdx;                                  //licznik w kierunku osi x
    int yIdx;                                  //licznik w kierunku osi y
    float texLeft;                          //lewa współrzędna tekstury
    float texBottom;                        //dolna współrzedna tekstury
    float texTop;                           //górna współrzedna tekstury
    float texRight;                         //prawa współrzędna tekstury

    glPushMatrix();
    glBindTexture(GL_TEXTURE_2D, texture);  //wybiera obiekt tekstury
    glBegin(GL_QUADS);                      //rysuje czworokąty

    for (xIdx = 0; xIdx < 36; xIdx++)
    {
        for (yIdx = 0; yIdx < 18; yIdx++)
        {
            //wyznacza współrzędne tekstury dla bieżącego czworokąta
            texLeft = float(xIdx) / 35.0f;
            texBottom = float(yIdx) / 18.0f;
            texRight = float(xIdx + 1) / 35.0f;
            texTop = float(yIdx + 1) / 18.0f;

            //lewy dolny wierzcholek
            glTexCoord2f(texLeft, texBottom);
            glVertex3f(flagPoints[xIdx][yIdx][0], flagPoints[xIdx][yIdx][1], flagPoints[xIdx][yIdx][2]);

            //prawy dolny wierzcholek
            glTexCoord2f(texRight, texBottom);
            glVertex3f(flagPoints[xIdx + 1][yIdx][0], flagPoints[xIdx + 1][yIdx][1], flagPoints[xIdx + 1][yIdx][2]);

            //prawy górny wierzcholek
            glTexCoord2f(texRight, texTop);
            glVertex3f(flagPoints[xIdx + 1][yIdx + 1][0], flagPoints[xIdx + 1][yIdx + 1][1], flagPoints[xIdx + 1][yIdx + 1][2]);

            //lewy górny wierzcholek
            glTexCoord2f(texLeft, texTop);
            glVertex3f(flagPoints[xIdx][yIdx + 1][0], flagPoints[xIdx][yIdx + 1][1], flagPoints[xIdx][yIdx + 1][2]);
        }
    }
    glEnd();

    //Udaje ruch flagi
    for (yIdx = 0; yIdx < 19; yIdx++)
    {
        wrapValue = flagPoints[35][yIdx][2];
        for (xIdx = 35; xIdx >= 0; xIdx--)
        {
            flagPoints[xIdx][yIdx][2] = flagPoints[xIdx - 1][yIdx][2];
        }
        flagPoints[0][yIdx][2] = wrapValue;
    }
    glPopMatrix();
}

//Przesunięcie układu współrzędnych
void Render()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();

    glTranslatef(-15.0f, -10.0f, -50.0f);               //wykonuje przesunięcie
    DrawFlag();                                         //rysuje flagę
    glFlush();
    SwapBuffers(g_HDC);                                 //przełącza bufory
}

//funkcja określająca format pikseli
void SetupPixelFormat(HDC hDC)
{
    int nPixelFormat;                                   //indeks formatu pikseli

    static PIXELFORMATDESCRIPTOR pfd = {
        sizeof(PIXELFORMATDESCRIPTOR),
        1,
        PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER,
        PFD_TYPE_RGBA,
        32,
        0,0,0,0,0,0,
        0,
        0,
        0,
        0,0,0,0,
        16,
        0,
        0,
        PFD_MAIN_PLANE,
        0,
        0,0,0
    };

    nPixelFormat = ChoosePixelFormat(hDC, &pfd);

    //określe format pikseli dla danego kontekstu urządzenia
    SetPixelFormat(hDC, nPixelFormat, &pfd);
}
//Procedura okienkowa
LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    static HGLRC hRC;
    static HDC hDC;
    int width, height;

    switch (message)
    {
    case WM_CREATE:
        hDC = GetDC(hwnd);
        g_HDC = hDC;
        SetupPixelFormat(hDC);

        hRC = wglCreateContext(hDC);
        wglMakeCurrent(hDC, hRC);
        return 0;
        break;

    case WM_CLOSE:
        wglMakeCurrent(hDC, NULL);
        wglDeleteContext(hRC);
        PostQuitMessage(0);
        return 0;
        break;

    case WM_SIZE:
        height = HIWORD(lParam);
        width = LOWORD(lParam);

        if (height == 0)
        {
            height = 1;
        }

        glViewport(0, 0, width, height);
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();

        gluPerspective(54.0f, (GLfloat)width / (GLfloat)height, 1.0f, 1000.0f);

        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();

        return 0;
        break;

    case WM_KEYDOWN:
        keyPressed[wParam] = true;
        return 0;
        break;

    case WM_KEYUP:
        keyPressed[wParam] = false;
        return 0;
        break;

    default:
        break;
    }
    return (DefWindowProc(hwnd, message, wParam, lParam));
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd) {
    WNDCLASSEX windowClass;
    HWND hwnd;
    MSG msg;
    bool done;
    DWORD dwExStyle;
    DWORD dwStyle;
    RECT windowRect;

    int width = 800;
    int height = 600;
    int bits = 32;

    windowRect.left = (long)0;
    windowRect.right = (long)width;
    windowRect.top = (long)0;
    windowRect.bottom = (long)height;

    windowClass.cbSize = sizeof(WNDCLASSEX);
    windowClass.style = CS_HREDRAW | CS_VREDRAW;
    windowClass.lpfnWndProc = WndProc;
    windowClass.cbClsExtra = 0;
    windowClass.cbWndExtra = 0;
    windowClass.hInstance = hInstance;
    windowClass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    windowClass.hCursor = LoadCursorW(NULL, IDC_ARROW);
    windowClass.hbrBackground = NULL;
    windowClass.lpszMenuName = NULL;
    windowClass.lpszClassName = L"Flaga";
    windowClass.hIconSm = LoadIconW(NULL, IDI_WINLOGO);

    if (!RegisterClassExW(&windowClass))
        return 0;

    if (fullScreen)
    {
        DEVMODE dmScreenSettings;
        memset(&dmScreenSettings, 0, sizeof(dmScreenSettings));
        dmScreenSettings.dmSize = sizeof(dmScreenSettings);
        dmScreenSettings.dmPelsWidth = width;
        dmScreenSettings.dmPelsHeight = height;
        dmScreenSettings.dmBitsPerPel = bits;
        dmScreenSettings.dmFields = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;

        if (ChangeDisplaySettings(&dmScreenSettings, CDS_FULLSCREEN) != DISP_CHANGE_SUCCESSFUL)
        {
            MessageBox(NULL, L"Display mode failed", NULL, MB_OK);
            fullScreen = false;
        }
    }
    if (fullScreen)
    {
        dwExStyle = WS_EX_APPWINDOW;
        dwStyle = WS_POPUP;
        ShowCursor(false);
    }
    else
    {
        dwExStyle = WS_EX_APPWINDOW | WS_EX_WINDOWEDGE;
        dwStyle = WS_OVERLAPPEDWINDOW;
    }

    AdjustWindowRectEx(&windowRect, dwStyle, false, dwExStyle);

    //tworzenia okna
    hwnd = CreateWindowExW(NULL,
        L"Flaga",
        L"Falowania flagi",
        dwStyle | WS_CLIPCHILDREN | WS_CLIPSIBLINGS,
        0, 0,
        windowRect.right - windowRect.left,
        windowRect.bottom - windowRect.top,
        NULL,
        NULL,
        hInstance,
        NULL);
    if (!hwnd)
        return 0;

    ShowWindow(hwnd, SW_SHOW);
    UpdateWindow(hwnd);

    done = false;
    Initialize();

    while (!done)
    {
        PeekMessage(&msg, hwnd, NULL, NULL, PM_REMOVE);

        if (msg.message == WM_QUIT)
        {
            done = true;
        }
        else
        {
            if (keyPressed[VK_ESCAPE])
                done = true;
            else
            {
                Render();
                TranslateMessage(&msg);
                DispatchMessageW(&msg);
            }
        }
    }
    free(bitmapData);

    if (fullScreen)
    {
        ChangeDisplaySettings(NULL, 0);
        ShowCursor(true);
    }
}