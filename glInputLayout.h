#ifndef GLINPUTLAYOUT_H__
#define GLINPUTLAYOUT_H__

#include <vector>
#include <string>

#include <GL/glew.h>

#include "glEnums.h"

struct InputLayout
{
    GLint size;
    GLEnums::DATA_TYPE type;
    GLboolean normalized;
    GLsizei stride;
    size_t offset;
};

struct GLInputLayout
{
    friend class GLDrawBinds;

    GLInputLayout();
    ~GLInputLayout();

    void AddInputLayout(GLint location, GLint size, GLEnums::DATA_TYPE type, GLboolean normalized, GLsizei stride, size_t offset);
    void AddInputLayout(const std::string& name, GLint size, GLEnums::DATA_TYPE type, GLboolean normalized, GLsizei stride, size_t offset);

    void AddDefaultInputLayout(GLint location);
    void AddDefaultInputLayout(const std::string& name);

private:
    std::vector<std::pair<std::string, InputLayout>> namedInputLayouts;
    size_t vertexBuffer;
};

#endif // GLINPUTLAYOUT_H__
