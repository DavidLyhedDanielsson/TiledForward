#ifndef GLDYNAMICBUFFER_H__
#define GLDYNAMICBUFFER_H__

#include <cstdio>

class GLDynamicBuffer
{
public:
    GLDynamicBuffer()
    { }
    ~GLDynamicBuffer()
    { }

    virtual size_t GetTotalSize() const = 0;
    virtual void UploadData(void* location) const = 0;
protected:
private:
};

#endif // GLDYNAMICBUFFER_H__
