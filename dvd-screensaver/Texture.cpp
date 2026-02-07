#include "Texture.h"


HMODULE getCurrentModule()
{
    HMODULE hModule = NULL;
    GetModuleHandleEx(
        GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS,
        (LPCTSTR)getCurrentModule,
        &hModule);
    return hModule;
}

Texture::Texture(int pResId)
{
    init(pResId);
}

void Texture::init(int pResId)
{
    resourceId = pResId;
    texId = LoadTextureFromResource(resourceId);
}

GLuint Texture::getTexture()
{
	return texId;
}

GLuint Texture::LoadTextureFromResource(int resourceID)
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
