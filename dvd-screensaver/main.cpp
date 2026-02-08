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


// Screen Saver Procedure
LRESULT WINAPI ScreenSaverProc(HWND hWnd, UINT message,
    WPARAM wParam, LPARAM lParam)
{
    //static RECT rect;
    static Renderer renderer;

    switch (message) {

    case WM_CREATE:

        static LONG style = GetWindowLong(hWnd, GWL_STYLE);
        static int width, height;

        if (style & WS_CHILD) // preview
        {
            static RECT rc;
            GetClientRect(hWnd, &rc);
            width = rc.right - rc.left;
            height = rc.bottom - rc.top;
        }
        else { //fulscr
            //GetClientRect(hWnd, &rect);
            // Virtual screen (multiple monitors)
            static int virtX = GetSystemMetrics(SM_XVIRTUALSCREEN);
            static int virtY = GetSystemMetrics(SM_YVIRTUALSCREEN);
            static int virtW = GetSystemMetrics(SM_CXVIRTUALSCREEN);
            static int virtH = GetSystemMetrics(SM_CYVIRTUALSCREEN);

            width = virtW, height = virtH;

            SetWindowPos(
                hWnd,
                nullptr,
                virtX, virtY,
                virtW, virtH,
                SWP_NOZORDER | SWP_NOACTIVATE
            );
        }

        // setup OpenGL, then animation
        renderer.init(hWnd);
        renderer.SetupAnimation(width, height);
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