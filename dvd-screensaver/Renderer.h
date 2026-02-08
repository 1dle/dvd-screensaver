#pragma once
#include <windows.h>
#include  <GL/gl.h>
#include <GL/glu.h>
#include <chrono>
#include "Texture.h"
#include "resource1.h"
#include "Item.h"
#include <array>

#pragma comment(lib, "dwmapi.lib")
#pragma comment(lib, "opengl32.lib")
#pragma comment(lib, "glu32.lib")
using namespace std::chrono;

constexpr float WORLD_HEIGHT = 20.0f;
constexpr double TARGET_FPS = 60.0;
constexpr double TARGET_FRAME_TIME = 1.0 / TARGET_FPS;

constexpr std::array<std::array<GLfloat, 3>, 6> COLORS{ {
{{0.0f, 1.0f, 0.0f}},
{{1.0f, 0.0f, 0.0f}},
{{0.0f, 0.0f, 1.0f}},
{{0.5f, 0.5f, 0.0f}},
{{0.4f, 0.0f, 0.65f}},
{{0.1f, 0.9f, 0.9f}}
} };

class Renderer
{
public:
	struct GLContext {
		HDC dc;
		HGLRC rc;
	};
	Item i;
	GLContext glc;
	int colorIndex = 0;
	Renderer() = default;
	void init(HWND &pHwnd);
	void dispose(HWND& pHwnd);
	void SetupAnimation(int virtW, int virtH);
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

