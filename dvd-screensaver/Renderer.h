#pragma once
#include <windows.h>
#include  <GL/gl.h>
#include <GL/glu.h>
#include <chrono>
#include "Texture.h"
#include "resource1.h"
#include "Item.h"

#pragma comment(lib, "dwmapi.lib")
#pragma comment(lib, "opengl32.lib")
#pragma comment(lib, "glu32.lib")
using namespace std::chrono;

const int NCOLOR = 6;

class Renderer
{
public:
	int colorIndex = 0;
	const GLfloat COLORS[NCOLOR][3] = {
		{0.0f, 1.0f, 0.0f},
		{1.0f, 0.0f, 0.0f},
		{0.0f, 0.0f, 1.0f},
		{0.5f, 0.5f, 0.0f},
		{0.4f, 0.0f, 0.65f},
		{0.1f, 0.9f, 0.9f}
	};

	struct GLContext {
		HDC dc;
		HGLRC rc;
	};
	Item i;
	GLContext glc;

	Renderer() = default;
	void init(HWND &pHwnd);
	void dispose(HWND& pHwnd);
	void SetupAnimation(int monW, int virtW, int virtH);
	void RenderFrame();
private:
	//work engine
	//int Width, Height;
	float Aspect;
	float camL, camR, camB, camT;
    steady_clock::time_point stc_prev;

	Texture sprite;
	void InitGL(HWND &pHwnd);
	void CloseGL(HWND& phWnd);

};

