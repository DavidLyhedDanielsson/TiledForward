#ifndef GLPIXELSHADER_H__
#define GLPIXELSHADER_H__

#include "glShader.h"

class GLPixelShader
        : public GLShader
{
public:
    GLPixelShader();
    ~GLPixelShader();

    bool Load(const std::string& path);
    bool Load(const std::string& path, const std::string& outBuffer);

protected:
private:
};

#endif // GLPIXELSHADER_H__
