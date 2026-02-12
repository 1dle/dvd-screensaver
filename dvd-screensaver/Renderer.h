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

static const std::array<std::array<float, 3>, 8> COLORS =
{
	std::array<float,3>{0.45f, 0.14f, 0.14f},
	std::array<float,3>{0.48f, 0.32f, 0.14f},
	std::array<float,3>{0.45f, 0.45f, 0.14f},
	std::array<float,3>{0.14f, 0.48f, 0.14f},
	std::array<float,3>{0.14f, 0.40f, 0.48f},
	std::array<float,3>{0.14f, 0.20f, 0.48f},
	std::array<float,3>{0.35f, 0.14f, 0.48f},
	std::array<float,3>{0.45f, 0.14f, 0.40f}
};




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

