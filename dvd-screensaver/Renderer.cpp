#include "Renderer.h"



void Renderer::init(HWND &pHwnd)
{
    InitGL(pHwnd);
    sprite.init(IDB_PNG1);
}

void Renderer::dispose(HWND& pHwnd)
{
    CloseGL(pHwnd);
}

void Renderer::InitGL(HWND &pHwnd)
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

    glc.dc = GetDC(pHwnd);

    int i = ChoosePixelFormat(glc.dc, &pfd);
    SetPixelFormat(glc.dc, i, &pfd);

    glc.rc = wglCreateContext(glc.dc);
    wglMakeCurrent(glc.dc, glc.rc);
}

void Renderer::CloseGL(HWND& phWnd)
{
    wglMakeCurrent(NULL, NULL);
    wglDeleteContext(glc.rc);

    ReleaseDC(phWnd, glc.dc);
}

void Renderer::SetupAnimation(int monW, int virtW, int virtH)
{
    Aspect = float(virtW) / virtH;
    float unit = monW / 10.0;
    camL = (-unit * Aspect);
    camR = -camL;
    camB = -unit;
    camT = -camB;

    //window resizing stuff
    glViewport(0, 0, (GLsizei)virtW, (GLsizei)virtH);

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
    stc_prev = steady_clock::now();
}

void Renderer::RenderFrame()
{
    steady_clock::time_point stc_now = steady_clock::now();
    duration<double> delt = duration_cast<duration<double>>(stc_now - stc_prev);
    stc_prev = stc_now;

    i.Update(delt.count(), camL, camR, camB, camT);
    if (i.HitX && i.HitY)
    {
        glColor3f(246.0f / 255, 162.0f / 255, 23.0f / 255);
    }
    else if (i.HitX || i.HitY)
    {
        glColor3fv(COLORS[colorIndex++ % NCOLOR]);
    }

    glClearColor(0, 0, 0, 0.15f);
    glClear(GL_COLOR_BUFFER_BIT);
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glPushMatrix();
       
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, sprite.getTexture());

    glTranslatef(i.X, i.Y, 0);
    glBegin(GL_QUADS);
    glTexCoord2f(0, 1); glVertex2f(-i.Width / 2, i.Height / 2);
    glTexCoord2f(0, 0); glVertex2f(-i.Width / 2, -i.Height / 2);
    glTexCoord2f(1, 0); glVertex2f(i.Width / 2, -i.Height / 2);
    glTexCoord2f(1, 1); glVertex2f(i.Width / 2, i.Height / 2);
    glEnd();
    glPopMatrix();
    glBindTexture(GL_TEXTURE_2D, 0);
    glDisable(GL_TEXTURE_2D);

    SwapBuffers(glc.dc);
}

