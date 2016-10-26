#ifndef GLVERTEXSHADER_H__
#define GLVERTEXSHADER_H__

#include "glShader.h"

#include <vector>

class GLVertexShader
        : public GLShader
{
public:
    GLVertexShader();
    ~GLVertexShader();

    bool Load(const std::string& path);

protected:
private:
};

#endif // GLVERTEXSHADER_H__
