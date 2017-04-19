#include "lightCull.h"

LightCull::LightCull()
{}

LightCull::~LightCull()
{}

void LightCull::InitShaderConstants(int screenWidth, int screenHeight)
{
    this->screenWidth = screenWidth;
    this->screenHeight = screenHeight;

    glGenQueries(1, &timeQuery);
}

int LightCull::GetTileCountX() const
{
    return -1;
}

void LightCull::StartTimeQuery()
{
    glBeginQuery(GL_TIME_ELAPSED, timeQuery);

}

GLuint64 LightCull::StopTimeQuery()
{
    glEndQuery(GL_TIME_ELAPSED);

    GLint timeAvailable = 0;
    while(!timeAvailable)
    {
        glGetQueryObjectiv(timeQuery,  GL_QUERY_RESULT_AVAILABLE, &timeAvailable);
        std::this_thread::sleep_for(std::chrono::nanoseconds(500));
    }

    GLuint64 time;
    glGetQueryObjectui64v(timeQuery, GL_QUERY_RESULT, &time);

    return time;
}
