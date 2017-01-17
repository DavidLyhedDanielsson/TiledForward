#ifndef GLBUFFER_H__
#define GLBUFFER_H__

#include "glBufferBase.h"

#include <GL/glew.h>

class GLBuffer
        : public GLBufferBase
{
public:
    GLBuffer();
    ~GLBuffer();

    virtual void Update(const void* data, size_t size) override;
protected:
private:
};

#endif // GLBUFFER_H__
