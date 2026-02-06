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
#include <chrono>
#include <cmath>

//LIBRARIES
#pragma comment(lib, "ScrnSavw.lib")
#pragma comment(lib, "opengl32.lib")
#pragma comment(lib, "glu32.lib")
#pragma comment(lib, "comctl32.lib")
#pragma comment (lib,"Gdiplus.lib")
#pragma comment(lib, "dwmapi.lib")
#pragma comment(lib, "Shlwapi.lib")
#pragma comment(lib, "winmm.lib")

#define TIMER 1


void InitGL(HWND hWnd, HDC& hDC, HGLRC& hRC);
void CloseGL(HWND hWnd, HDC hDC, HGLRC hRC);
void SetupAnimation(int Width, int Height);
void CleanupAnimation();
void OnTimer(HDC hDC);

int Width, Height; //globals for size of screen
float Aspect;
float camL, camR, camB, camT;

// fps
static DWORD rec_time;
static DWORD prev_time;
static int count_frame;
static int count_fps;
std::chrono::steady_clock::time_point stc_prev;

//settings

static int waitValue = 15;

void dbgprint(float v) {
    wchar_t text_buffer[20] = { 0 }; //temporary buffer
    swprintf(text_buffer, _countof(text_buffer), L"%f\n", v); // convert
    OutputDebugString(text_buffer); // print
}



//////////////////////////////////////////////////
////   INFRASTRUCTURE -- THE THREE FUNCTIONS   ///
//////////////////////////////////////////////////
static void initCountFps(void);
static void closeCountFps(void);
static void countFps(void);
// ========================================
// Init count FPS work
static void initCountFps(void)
{
    timeBeginPeriod(1);
    rec_time = timeGetTime();
    prev_time = rec_time;
    count_fps = 0;
    count_frame = 0;
}

// Close count FPS
static void closeCountFps(void)
{
    timeEndPeriod(1);
}

// count FPS
static void countFps(void)
{
    count_frame++;
    DWORD t = timeGetTime() - rec_time;

    if (t >= 1000)
    {
        rec_time += 1000;
        count_fps = count_frame;
        count_frame = 0;
    }
    else if (t < 0)
    {
        rec_time = timeGetTime();
        count_fps = 0;
        count_frame = 0;
    }
}

// Screen Saver Procedure
LRESULT WINAPI ScreenSaverProc(HWND hWnd, UINT message,
    WPARAM wParam, LPARAM lParam)
{
    static HDC hDC;
    static HGLRC hRC;
    static RECT rect;
    SetPriorityClass(GetCurrentProcess(), HIGH_PRIORITY_CLASS);
    //FreeConsole();

    switch (message) {

    case WM_CREATE:

        // get window dimensions
        GetClientRect(hWnd, &rect);
        Width = rect.right;
        Height = rect.bottom;
        Aspect = (Width + 0.0f) / Height;
        initCountFps();         // initialize FPS counter
        // setup OpenGL, then animation
        InitGL(hWnd, hDC, hRC);
        SetupAnimation(Width, Height);

        //set timer to tick every 10 ms
        //SetTimer(hWnd, TIMER, waitValue, NULL);
        return 0;

    case WM_DESTROY:
        closeCountFps();
        KillTimer(hWnd, TIMER);
        CleanupAnimation();
        CloseGL(hWnd, hDC, hRC);
        return 0;

    case WM_PAINT:
        OnTimer(hDC);       //animate!     
        InvalidateRect(hWnd, NULL, FALSE);
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
    float X = 200, Y = 0, speed = 60;
    float Width = 100, Height = 100;
    float XDir = 1, YDir = 1;
    int ColorCounter = -1;
    
    void Move(float delta) {
        int corner = 0;

        float dX = XDir * speed * delta;
        float dY = YDir * speed * delta;
        bool hitX = false, hitY = false;

        //X check
        X += dX;
        if (X - Width / 2 < camL || X + Width / 2 > camR) {
            XDir *= -1;
            X += XDir * fabs(dX); // bounce remainder
            hitX = true;
        }
        //Y check
        Y += dY;
        if (Y - Height / 2 < camB || Y + Height / 2 > camT) {
            YDir *= -1;
            Y += YDir * fabs(dY);
            hitY = true;
        }

        if (hitX && hitY) {
            glColor3f(246.0 / 255, 162.0 / 255, 23.0 / 255); //specific color when hits corner
        }
        else if (hitX || hitY) {
            if (++ColorCounter == NCOLOR) ColorCounter = 0;
            glColor3fv(COLORS[ColorCounter]);
        }

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

    PIXELFORMATDESCRIPTOR pfd =
    {
        sizeof(PIXELFORMATDESCRIPTOR), // size of this pfd
        1,                             // version number
        // support window, OpenGL, double bufferd
        PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER,
        PFD_TYPE_RGBA,  // RGBA type
        32,             // color
        0, 0,           // R
        0, 0,           // G
        0, 0,           // B
        0, 0,           // A
        0,              // accumulation buffer
        0, 0, 0, 0,     // accum bits ignored
        24,             // depth
        8,              // stencil buffer
        0,              // auxiliary buffer
        PFD_MAIN_PLANE, // main layer
        0,              // reserved
        0, 0, 0         // layermask, visiblemask, damagemask
    };

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
    float unit = Width / 10;
    camL = (-unit * Aspect);
    camR = -camL;
    camB = -unit;
    camT = -camB;

    //window resizing stuff
    glViewport(0, 0, (GLsizei)Width, (GLsizei)Height);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(camL, camR, camB, camT, -1, 1);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    //gluLookAt(0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);
    //camera xyz, the xyz to look at, and the up vector (+y is up)

    //background
    glClearColor(0.0, 0.0, 0.0, 0.0); //0.0s is black

    glColor3f(1.0, 1.0, 1.0); //white

    loadImage();
    //prev_time = timeGetTime();
    stc_prev = std::chrono::steady_clock::now();
}

//static GLfloat spin = 0;   //a global to keep track of the square's spinning
static float xTex[] = { 0, 0, 1, 1 };
static float yTex[] = { 1, 0, 0, 1 };

static float xvals[] = { -i.Width / 2, -i.Width / 2, i.Width / 2, i.Width / 2 };
static float yvals[] = { i.Height / 2, -i.Height / 2, -i.Height / 2, i.Height / 2 };

void OnTimer(HDC hDC) //increment and display
{
    std::chrono::steady_clock::time_point stc_now = std::chrono::steady_clock::now();
    std::chrono::duration<double> delt = std::chrono::duration_cast<std::chrono::duration<double>>(stc_now - stc_prev);
    stc_prev = stc_now;


    i.Move(delt.count());

    glClearColor(0, 0, 0, 0.15f);
    glClear(GL_COLOR_BUFFER_BIT);
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glPushMatrix();
    
    glTranslatef(i.X, i.Y, 0);
    
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glBegin(GL_POLYGON);
    for (int j = 0; j < 4; j++) {
        glTexCoord2f(xTex[j], yTex[j]);
        glVertex2f(xvals[j], yvals[j]);
    } 
    glEnd();

    glPopMatrix();
    glDisable(GL_TEXTURE_2D);
    //glFlush();
    
    //glColor4f(1, 1, 1, 1);           // set color
    /*glRasterPos3f(-1, 1.0, -1.08); // set postion
    {
        char buf[512];
        sprintf_s(buf, "%d FPS", count_fps);
        drawText(buf);
    }
    */
    SwapBuffers(hDC);

    countFps(); // calc FPS
}

void CleanupAnimation()
{
    //free memory
}