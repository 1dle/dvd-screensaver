#include <windows.h>
#include  <scrnsave.h>
#include <wincodec.h>
#include  <GL/gl.h>
#include <GL/glu.h>
#include <cstdio>
#include "resource1.h"
#include <gdiplus.h>
#include <Shlwapi.h>
#include <objidl.h>
//LIBRARIES
#pragma comment(lib, "ScrnSavw.lib")
#pragma comment(lib, "opengl32.lib")
#pragma comment(lib, "glu32.lib")
#pragma comment(lib, "comctl32.lib")
#pragma comment (lib,"Gdiplus.lib")
#pragma comment(lib, "dwmapi.lib")
#pragma comment(lib, "Shlwapi.lib")

#define TIMER 1

void InitGL(HWND hWnd, HDC& hDC, HGLRC& hRC);
void CloseGL(HWND hWnd, HDC hDC, HGLRC hRC);
void SetupAnimation(int Width, int Height);
void CleanupAnimation();
void OnTimer(HDC hDC);

int Width, Height; //globals for size of screen
float Aspect;
float camL, camR, camB, camT;
//////////////////////////////////////////////////
////   INFRASTRUCTURE -- THE THREE FUNCTIONS   ///
//////////////////////////////////////////////////


// Screen Saver Procedure
LRESULT WINAPI ScreenSaverProc(HWND hWnd, UINT message,
    WPARAM wParam, LPARAM lParam)
{
    static HDC hDC;
    static HGLRC hRC;
    static RECT rect;

    //FreeConsole();

    switch (message) {

    case WM_CREATE:

        // get window dimensions
        GetClientRect(hWnd, &rect);
        Width = rect.right;
        Height = rect.bottom;
        Aspect = (Width + 0.0f) / Height;

        // setup OpenGL, then animation
        InitGL(hWnd, hDC, hRC);
        SetupAnimation(Width, Height);

        //set timer to tick every 10 ms
        SetTimer(hWnd, TIMER, 2, NULL);
        return 0;

    case WM_DESTROY:
        KillTimer(hWnd, TIMER);
        CleanupAnimation();
        CloseGL(hWnd, hDC, hRC);
        return 0;

    case WM_TIMER:
        OnTimer(hDC);       //animate!      
        return 0;

    }

    return DefScreenSaverProc(
        hWnd, message, wParam, lParam);

}

bool bTumble = true;


BOOL WINAPI
ScreenSaverConfigureDialog(HWND hDlg, UINT message,
    WPARAM wParam, LPARAM lParam)
{
    return FALSE;
}

// needed for SCRNSAVE.LIB
BOOL WINAPI RegisterDialogClasses(HANDLE hInst)
{
    return TRUE;
}


/////////////////////////////////////////////////
////   INFRASTRUCTURE ENDS, SPECIFICS BEGIN   ///
////                                          ///
////    In a more complex scr, I'd put all    ///
////     the following into other files.      ///
/////////////////////////////////////////////////

static GLfloat step = 1;
const int NCOLOR = 6;

const GLfloat COLORS[NCOLOR][3] = {
    {0.0f, 1.0f, 0.0f},
    {1.0f, 0.0f, 0.0f},
    {0.0f, 0.0f, 1.0f},
    {0.5f, 0.5f, 0.0f},
    {0.4f, 0.0f, 0.65f},
    {0.1f, 0.9f, 0.9f}
};

struct Item {
    GLfloat X = 200, Y = 0, Step = 1;
    GLfloat Width = 100, Height = 100;
    GLfloat XDir = 1, YDir = 1;
    int ColorCounter = -1;

    void Move() {
        int corner = 0;
        if ((int)X < camL + Width / 2 || X + Width / 2 >(int)camR) {
            XDir *= -1;
            Collision();
            corner++;
        }
        //OK
        if (Y == camB + Height / 2 || Y + Height / 2 == camT) {
            YDir *= -1;
            Collision();
            corner++;
        }

        if (corner == 2) {
            ColorCounter -= 2;
            glColor3f(246.0/255, 162.0/255, 23.0/255); //specific color when hits corner
        }
        X += XDir * Step;
        Y += YDir * Step;
    }

    void Collision() {
        if (++ColorCounter == NCOLOR) ColorCounter = 0;
        glColor3fv(COLORS[ColorCounter]);

    }
};

static Item i;

HMODULE getCurrentModule()
{
    HMODULE hModule = NULL;
    GetModuleHandleEx(
        GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS,
        (LPCTSTR)getCurrentModule,
        &hModule);
    return hModule;
}

BITMAP loadImageFromResource(int resourceID)
{
    HBITMAP hbitmap = NULL;
    ULONG_PTR token;
    Gdiplus::GdiplusStartupInput tmp;
    Gdiplus::GdiplusStartup(&token, &tmp, NULL);
    if (auto hres = FindResource(getCurrentModule(), MAKEINTRESOURCE(resourceID), L"PNG"))
        if (auto size = SizeofResource(getCurrentModule(), hres))
            if (auto data = LockResource(LoadResource(getCurrentModule(), hres)))
                if (auto stream = SHCreateMemStream((BYTE*)data, size))
                {
                    Gdiplus::Bitmap bmp(stream);
                    stream->Release();
                    bmp.GetHBITMAP(Gdiplus::Color::Transparent, &hbitmap);
                }
    Gdiplus::GdiplusShutdown(token);
    BITMAP bitmap;
    //if (!hbitmap) return NULL;
    GetObject(hbitmap, sizeof(BITMAP), &bitmap);
    return bitmap;
}

void loadImage() {
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    BITMAP bmap = loadImageFromResource(IDB_PNG1);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    //4 channels. cant get the information from bitmap (at least i do not know how to.)
    gluBuild2DMipmaps(GL_TEXTURE_2D, 4, bmap.bmWidth, bmap.bmHeight,
        GL_RGBA, GL_UNSIGNED_BYTE, (const void*)bmap.bmBits);

    //glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, iW, iH, 0, GL_RGBA, GL_UNSIGNED_BYTE, loadedimg);
    //glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, bmap.bmWidth, bmap.bmHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, (const void*)bmap.bmBits);
    glBindTexture(GL_TEXTURE_2D, 0);
}

// Initialize OpenGL
static void InitGL(HWND hWnd, HDC& hDC, HGLRC& hRC)
{

    PIXELFORMATDESCRIPTOR pfd;
    ZeroMemory(&pfd, sizeof pfd);
    pfd.nSize = sizeof pfd;
    pfd.nVersion = 1;
    pfd.dwFlags = PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
    pfd.iPixelType = PFD_TYPE_RGBA;
    pfd.cColorBits = 24;

    hDC = GetDC(hWnd);

    int i = ChoosePixelFormat(hDC, &pfd);
    SetPixelFormat(hDC, i, &pfd);

    hRC = wglCreateContext(hDC);
    wglMakeCurrent(hDC, hRC);

}

// Shut down OpenGL
static void CloseGL(HWND hWnd, HDC hDC, HGLRC hRC)
{
    wglMakeCurrent(NULL, NULL);
    wglDeleteContext(hRC);

    ReleaseDC(hWnd, hDC);
}


void SetupAnimation(int Width, int Height)
{
    //window resizing stuff
    glViewport(0, 0, (GLsizei)Width, (GLsizei)Height);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    float unit = Width/10;
    camL = (-unit * Aspect);
    camR = -camL;
    camB = -unit;
    camT = -camB;


    glOrtho(camL, camR, camB, camT, 25.0, 75.0);
    glMatrixMode(GL_MODELVIEW);

    glLoadIdentity();
    gluLookAt(0.0, 0.0, 50.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);
    //camera xyz, the xyz to look at, and the up vector (+y is up)

    //background
    glClearColor(0.0, 0.0, 0.0, 0.0); //0.0s is black

    glColor3f(1.0, 1.0, 1.0); //white

    loadImage();

}

//static GLfloat spin = 0;   //a global to keep track of the square's spinning
static float xTex[] = { 0, 0, 1, 1 };
static float yTex[] = { 1, 0, 0, 1 };

static float xvals[] = { -i.Width / 2, -i.Width / 2, i.Width / 2, i.Width / 2 };
static float yvals[] = { i.Height / 2, -i.Height / 2, -i.Height / 2, i.Height / 2 };

void OnTimer(HDC hDC) //increment and display
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_TEXTURE_2D);
    glPushMatrix();

    i.Move();

    glTranslatef(i.X, i.Y, 0);

    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glBegin(GL_POLYGON);
    for (int j = 0; j < 4; j++) {
        glTexCoord2f(xTex[j], yTex[j]);
        glVertex2f(xvals[j], yvals[j]);
    } 
    glEnd();

    //glPopMatrix();

    glFlush();
    SwapBuffers(hDC);
    glPopMatrix();
    glDisable(GL_TEXTURE_2D);
}

void CleanupAnimation()
{
    //free memory
}
