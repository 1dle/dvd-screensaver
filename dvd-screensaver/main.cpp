#include <windows.h>
#include  <scrnsave.h>
#include  <GL/gl.h>
#include <GL/glu.h>
#include "resource1.h"
#include <chrono>
#include <cmath>
#include "Renderer.h"

//LIBRARIES
#pragma comment(lib, "ScrnSavw.lib")
#pragma comment(lib, "opengl32.lib")
#pragma comment(lib, "glu32.lib")
#pragma comment(lib, "comctl32.lib")


void dbgprint(float v) {
    wchar_t text_buffer[20] = { 0 }; //temporary buffer
    swprintf(text_buffer, _countof(text_buffer), L"%f\n", v); // convert
    OutputDebugString(text_buffer); // print
}

// Screen Saver Procedure
LRESULT WINAPI ScreenSaverProc(HWND hWnd, UINT message,
    WPARAM wParam, LPARAM lParam)
{
    static RECT rect;
    static Renderer renderer;

    switch (message) {

    case WM_CREATE:
        // get window dimensions
        //GetClientRect(hWnd, &rect);
        // Virtual screen (all monitors)
        static int virtX = GetSystemMetrics(SM_XVIRTUALSCREEN);
        static int virtY = GetSystemMetrics(SM_YVIRTUALSCREEN);
        static int virtW = GetSystemMetrics(SM_CXVIRTUALSCREEN);
        static int virtH = GetSystemMetrics(SM_CYVIRTUALSCREEN);

        // Primary monitor (reference monitor)
        static int monW = GetSystemMetrics(SM_CXSCREEN);
        static int monH = GetSystemMetrics(SM_CYSCREEN);
         
        SetWindowPos(
            hWnd,
            nullptr,
            virtX, virtY,
            virtW, virtH,
            SWP_NOZORDER | SWP_NOACTIVATE
        );

        // setup OpenGL, then animation
        renderer.init(hWnd);
        renderer.SetupAnimation(monW, virtW, virtH);
        return 0;

    case WM_DESTROY:
        renderer.dispose(hWnd);
        return 0;

    case WM_PAINT:
        renderer.RenderFrame();       //animate!     
        InvalidateRect(hWnd, NULL, FALSE);
        return 0;
    }
    return DefScreenSaverProc(
        hWnd, message, wParam, lParam);
}
// needed for SCRNSAVE.LIB
BOOL WINAPI
ScreenSaverConfigureDialog(HWND hDlg, UINT message,
    WPARAM wParam, LPARAM lParam)
{
    return FALSE;
}

BOOL WINAPI RegisterDialogClasses(HANDLE hInst)
{
    return TRUE;
}