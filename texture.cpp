#include "texture.h"

Texture::Texture()
    : texture(0)
{}

Texture::~Texture()
{}

void Texture::CreateFromMemory(char* imageData, int width, int height)
{
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, imageData);

    glBindTexture(GL_TEXTURE_2D, 0);
}

GLuint Texture::GetTexture() const
{
    return texture;
}

void Texture::Bind()
{
    glBindTexture(GL_TEXTURE_2D, texture);
}

void Texture::Unbind()
{
    glBindTexture(GL_TEXTURE_2D, 0);
}
