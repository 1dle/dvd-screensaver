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

    //set vsync
   
    /*
    typedef BOOL(WINAPI* PFNWGLSWAPINTERVALEXTPROC)(int);

    PFNWGLSWAPINTERVALEXTPROC wglSwapIntervalEXT =
        (PFNWGLSWAPINTERVALEXTPROC)wglGetProcAddress("wglSwapIntervalEXT");

    if (wglSwapIntervalEXT) {
        wglSwapIntervalEXT(1); // enable vsync
    }
    */
}

void Renderer::CloseGL(HWND& phWnd)
{
    wglMakeCurrent(NULL, NULL);
    wglDeleteContext(glc.rc);

    ReleaseDC(phWnd, glc.dc);
}

void Renderer::SetupAnimation(int virtW, int virtH)
{
    Aspect = float(virtW) / virtH;

    camB = -WORLD_HEIGHT * 0.5f;
    camT = WORLD_HEIGHT * 0.5f;

    camL = camB * Aspect;
    camR = camT * Aspect;

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
    static const double FIXED_DT = 1.0 / 60.0;
    static double accumulator = 0.0;

    // Color smoothing
    static float curColor[3] = { 1.0f, 1.0f, 1.0f };
    static float targetColor[3] = { 1.0f, 1.0f, 1.0f };
    static int lastColorIndex = 0;

    auto Lerp = [](float a, float b, float t)
        {
            return a + (b - a) * t;
        };

    // Timing
    steady_clock::time_point now = steady_clock::now();
    duration<double> dt = duration_cast<duration<double>>(now - stc_prev);
    stc_prev = now;

    accumulator += dt.count();
    if (accumulator > 0.25)
        accumulator = 0.25;

    // Save previous position for interpolation
    float prevX = i.X;
    float prevY = i.Y;

    // Fixed physics updates
    while (accumulator >= FIXED_DT)
    {
        i.Update(FIXED_DT, camL, camR, camB, camT);

        if (i.HitX || i.HitY)
        {
            if (i.HitX && i.HitY)
            {
                targetColor[0] = 246.0f / 255.0f;
                targetColor[1] = 162.0f / 255.0f;
                targetColor[2] = 23.0f / 255.0f;
            }
            else
            {
                lastColorIndex = (lastColorIndex + 1) % std::size(COLORS);
                targetColor[0] = COLORS[lastColorIndex][0];
                targetColor[1] = COLORS[lastColorIndex][1];
                targetColor[2] = COLORS[lastColorIndex][2];
            }
        }

        accumulator -= FIXED_DT;
    }

    // Interpolation
    float alpha = float(accumulator / FIXED_DT);
    float renderX = prevX + (i.X - prevX) * alpha;
    float renderY = prevY + (i.Y - prevY) * alpha;

    // Smooth color transition
    float t = float(dt.count() * 9.0f);
    if (t > 1.0f) t = 1.0f;

    curColor[0] = Lerp(curColor[0], targetColor[0], t);
    curColor[1] = Lerp(curColor[1], targetColor[1], t);
    curColor[2] = Lerp(curColor[2], targetColor[2], t);

    // CRT flicker
    //float flicker = 0.92f + (rand() % 20) / 100.0f; // 0.92..1.12

    // Jitter (subtle)
    //float jitterX = ((rand() % 3) - 1) * 0.02f;
    //float jitterY = ((rand() % 3) - 1) * 0.02f;

    glClearColor(0, 0, 0, 0.15f);
    glClear(GL_COLOR_BUFFER_BIT);

    glEnable(GL_TEXTURE_2D);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glPushMatrix();

    glBindTexture(GL_TEXTURE_2D, sprite.getTexture());

    // Apply jitter + interpolation
    //glTranslatef(renderX + jitterX, renderY + jitterY, 0);
    glTranslatef(renderX, renderY, 0);

    // Apply flicker to color
    //glColor3f(curColor[0] * flicker, curColor[1] * flicker, curColor[2] * flicker);
    glColor3f(curColor[0], curColor[1], curColor[2] );

    glBegin(GL_QUADS);
    glTexCoord2f(0, 1); glVertex2f(-i.Width / 2, i.Height / 2);
    glTexCoord2f(0, 0); glVertex2f(-i.Width / 2, -i.Height / 2);
    glTexCoord2f(1, 0); glVertex2f(i.Width / 2, -i.Height / 2);
    glTexCoord2f(1, 1); glVertex2f(i.Width / 2, i.Height / 2);
    glEnd();

    glPopMatrix();

    glBindTexture(GL_TEXTURE_2D, 0);
    glDisable(GL_TEXTURE_2D);

    // --- scanlines ---
    //glColor4f(0, 0, 0, 0.06f);  // transparency
    //glBegin(GL_LINES);
    //for (float y = camB; y <= camT; y += 0.25f)
    //{
    //   glVertex2f(camL, y);
    //  glVertex2f(camR, y);
    //}
    //glEnd();

    SwapBuffers(glc.dc);
}

