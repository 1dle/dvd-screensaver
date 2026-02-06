#include <windows.h>
#include  <scrnsave.h>
#include <wincodec.h>
#include  <GL/gl.h>
#include <GL/glu.h>
#include <cstdio>
#include "resource1.h"
#include <Shlwapi.h>
#include <objidl.h>
#include <chrono>
#include <cmath>
#include "stb_image.h"

//LIBRARIES
#pragma comment(lib, "ScrnSavw.lib")
#pragma comment(lib, "opengl32.lib")
#pragma comment(lib, "glu32.lib")
#pragma comment(lib, "comctl32.lib")
#pragma comment (lib,"Gdiplus.lib")
#pragma comment(lib, "dwmapi.lib")
#pragma comment(lib, "Shlwapi.lib")
#pragma comment(lib, "winmm.lib")



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
GLuint LoadTextureFromResource(int resourceID);
GLuint gTexID;
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
        gTexID = LoadTextureFromResource(IDB_PNG1);
        SetupAnimation(Width, Height);

        //set timer to tick every 10 ms
        //SetTimer(hWnd, TIMER, waitValue, NULL);
        return 0;

    case WM_DESTROY:
        closeCountFps();
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

GLuint LoadTextureFromResource(int resourceID)
{
    HRSRC hRes = FindResource(getCurrentModule(),
        MAKEINTRESOURCE(resourceID), L"PNG");
    if (!hRes) return 0;

    DWORD size = SizeofResource(getCurrentModule(), hRes);
    HGLOBAL hData = LoadResource(getCurrentModule(), hRes);
    void* data = LockResource(hData);

    int width, height, channels;
    unsigned char* pixels = stbi_load_from_memory(
        (unsigned char*)data,
        (int)size,
        &width, &height,
        &channels,
        STBI_rgb_alpha);

    if (!pixels) return 0;

    GLuint texID;
    glGenTextures(1, &texID);
    glBindTexture(GL_TEXTURE_2D, texID);

    glTexImage2D(
        GL_TEXTURE_2D,
        0,
        GL_RGBA,
        width,
        height,
        0,
        GL_RGBA,
        GL_UNSIGNED_BYTE,
        pixels);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    stbi_image_free(pixels);
    glBindTexture(GL_TEXTURE_2D, 0);

    return texID;
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

    //prev_time = timeGetTime();
    stc_prev = std::chrono::steady_clock::now();
}

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
    
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, gTexID);

    glBegin(GL_QUADS);
    glTexCoord2f(0, 1); glVertex2f(-i.Width / 2, i.Height / 2);
    glTexCoord2f(0, 0); glVertex2f(-i.Width / 2, -i.Height / 2);
    glTexCoord2f(1, 0); glVertex2f(i.Width / 2, -i.Height / 2);
    glTexCoord2f(1, 1); glVertex2f(i.Width / 2, i.Height / 2);
    glEnd();
    glPopMatrix();
    glBindTexture(GL_TEXTURE_2D, 0);
    glDisable(GL_TEXTURE_2D);
    
    SwapBuffers(hDC);

    countFps(); // calc FPS
}

void CleanupAnimation()
{
    //free memory
}