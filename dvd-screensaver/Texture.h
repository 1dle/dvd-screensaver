#pragma once
#include <windows.h>
#include  <GL/gl.h>
#include <GL/glu.h>
#include "stb_image.h"
#pragma comment(lib, "winmm.lib")
#pragma comment(lib, "opengl32.lib")

class Texture
{
public:
	Texture() {};
	Texture(int pResId);
	void init(int pResId);
	GLuint getTexture();
private:
	int resourceId;
	GLuint texId;
	GLuint LoadTextureFromResource(int resourceID);
};

