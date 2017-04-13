#ifndef LIGHTMANAGER_H__
#define LIGHTMANAGER_H__

#include <cstring>
#include "console/console.h"
#include "gl/glDynamicBuffer.h"
#include "primitiveDrawer.h"

struct LightData
{
    glm::vec3 position;
    float strength;
    glm::vec3 color;
    float lifetime;
};

class LightsBuffer
        : public GLDynamicBuffer
{
public:
    size_t GetTotalSize() const override
    {
        return sizeof(glm::vec3) + sizeof(float) + sizeof(LightData) * lights.size();
    }

    void UploadData(void* location) const override
    {
        char* locationChar = (char*)location;

        std::memcpy(locationChar + sizeof(padding), &ambientStrength, sizeof(float));
        std::memcpy(locationChar + sizeof(padding) + sizeof(ambientStrength), &lights[0], sizeof(LightData) * lights.size());
    }

    glm::vec3 padding;
    float ambientStrength;
    std::vector<LightData> lights;
};

class LightManager
{
public:
    LightManager();
    ~LightManager();

    void AddConsoleCommands(Console& console);

    void Update(Timer& deltaTimer, PrimitiveDrawer& primitiveDrawer);

    LightsBuffer& GetLightsBuffer();
protected:
private:
    enum LIGHT_POSITION_STRATEGY
    {
        RANDOM
        , CLUSTERED
    };

    int lightCount;

    int lightClusters;
    float lightClusterRadius;

    LightsBuffer lightsBuffer;

    LIGHT_POSITION_STRATEGY lightPositionStrategy;
    std::vector<glm::vec3> clusterPositions;

    bool freezeLights;
    bool drawLightSpheres;

    const float LIGHT_DEFAULT_AMBIENT;

    float lightMinStrength = 50.0f;
    float lightMaxStrength = 50.0f;
    float lightLifetime = 2500.0f;

    const float CLUSTER_MAX_X = 0.0f;
    const float CLUSTER_MAX_Y = 0.0f;
    const float CLUSTER_MIN_Y = 0.5f;
    const float CLUSTER_MAX_Z = 0.0f;

    //const float CLUSTER_MAX_X = 10.0f;
    //const float CLUSTER_MAX_Y = 8.0f;
    //const float CLUSTER_MIN_Y = 0.25f;
    //const float CLUSTER_MAX_Z = 6.0f;

//#define SINGLE_POSITION
#ifdef SINGLE_POSITION
    const float LIGHT_RANGE_X = 0.0f;
    const float LIGHT_MAX_Y = 0.0f;
    const float LIGHT_RANGE_Z = 0.0f;
#else
    //const float LIGHT_RANGE_X = 14.0f;
    //const float LIGHT_MAX_Y = 2.0f;
    //const float LIGHT_RANGE_Z = 6.0f;

    const float LIGHT_RANGE_X = 14.0f;
    const float LIGHT_MAX_Y = 8.0f;
    const float LIGHT_RANGE_Z = 6.0f;
#endif

    LightData GetNewLight();
    LightData GetRandomLight();
    glm::vec3 GetRandomLightPosition();
    float GetLightRadius(float lifetime);
};

#endif // LIGHTMANAGER_H__
