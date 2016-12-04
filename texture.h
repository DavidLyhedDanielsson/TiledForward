#ifndef TEXTURE_H__
#define TEXTURE_H__

#include <GL/glew.h>
#include <vector>

class Texture
{
public:
    Texture();
    ~Texture();

    void CreateFromMemory(char* imageData, int width, int height);

    GLuint GetTexture() const;

    void Bind();
    void Unbind();

protected:
private:

    GLuint texture;
};

#endif // TEXTURE_H__
