#include <glm/gtc/constants.hpp>
#include "lightManager.h"
#include "console/commandGetSet.h"
#include "console/commandCallMethod.h"
#include "primitiveDrawer.h"

LightManager::LightManager()
        : lightCount(3)
          , lightClusters(1)
          , lightClusterRadius(1.0f)
          , lightPositionStrategy(RANDOM)
          , LIGHT_DEFAULT_AMBIENT(0.0f)
          , freezeLights(false)
          , drawLightSpheres(false)
{
    lightsBuffer.lights.reserve(lightCount);

    lightsBuffer.padding = glm::vec3(1.0f, 1.2f, 1.23f);
    lightsBuffer.ambientStrength = LIGHT_DEFAULT_AMBIENT;

    for(int i = 0; i < lightClusters; ++i)
    {
        float xPos = ((rand() / (float)RAND_MAX) * 2.0f - 1.0f) * CLUSTER_MAX_X;
        float yPos = ((rand() / (float)RAND_MAX) * 2.0f - 1.0f) * (CLUSTER_MAX_Y - CLUSTER_MIN_Y) + CLUSTER_MIN_Y;
        float zPos = ((rand() / (float)RAND_MAX) * 2.0f - 1.0f) * CLUSTER_MAX_Z;
        clusterPositions.push_back(glm::vec3(xPos, yPos, zPos));
    }

    for(int i = 0; i < lightCount; ++i)
    {
        lightsBuffer.lights.push_back(GetRandomLight());
        if(lightPositionStrategy == CLUSTERED)
            lightsBuffer.lights[i].position += clusterPositions[i / (lightCount / lightClusters)];
    }
}

LightManager::~LightManager()
{}

void LightManager::AddConsoleCommands(Console& console)
{
    console.AddCommand(new CommandGetSet<bool>("light_freeze", &freezeLights));
    console.AddCommand(new CommandGetSet<bool>("light_drawSpheres", &drawLightSpheres));
    console.AddCommand(new CommandGetSet<float>("light_minStrength", &lightMinStrength));
    console.AddCommand(new CommandGetSet<float>("light_maxStrength", &lightMaxStrength));
    console.AddCommand(new CommandGetSet<float>("light_lifetime", &lightLifetime));

    console.AddCommand(new CommandCallMethod("light_PositionStrategy"
                                             , [&](const std::vector<Argument>& args)
            {
                if(args.size() != 1)
                    return Argument("Expected 1 argument");

                std::string arg = args.front().value;

                bool validArg = true;
                bool changed = false;

                if(arg == "random"
                   && lightPositionStrategy != RANDOM)
                {
                    lightPositionStrategy = RANDOM;
                    changed = true;
                }
                else if(arg == "clustered"
                        && lightPositionStrategy != CLUSTERED)
                {
                    lightPositionStrategy = CLUSTERED;
                    changed = true;
                }
                else
                    validArg = false;

                if(validArg)
                {


                    return Argument("Light position strategy updated");
                }
                else
                    return Argument("Invalid argument");
            }
                                             , FORCE_STRING_ARGUMENTS::PER_ARGUMENT
                                             , AUTOCOMPLETE_TYPE::ONLY_CUSTOM
                                             , "random", "clustered"
    ));

    console.AddCommand(new CommandCallMethod("light_lightCount"
                                             , [&](const std::vector<Argument>& args)
            {
                if(args.size() == 0)
                    return Argument("lightCount = " + std::to_string(lightCount));
                else if(args.size() != 1)
                    return Argument("Needs 1 parameter");

                int newCount = std::stoi(args.front().value);

                if(newCount > lightCount)
                {
                    lightsBuffer.lights.reserve(newCount);
                    for(int i = 0; i < newCount - lightCount; ++i)
                        lightsBuffer.lights.push_back(GetRandomLight());
                }
                else
                {
                    lightsBuffer.lights.resize(newCount);
                    for(int i = 0; i < newCount; ++i)
                        lightsBuffer.lights[i] = GetRandomLight();
                }

                lightCount = newCount;

                return Argument("lightCount updated to " + std::to_string(lightCount));
            }
    ));

    console.AddCommand(new CommandCallMethod("light_strength"
                                             , [&](const std::vector<Argument>& args)
            {
                if(args.size() != 2)
                    return Argument("Needs 2 parameter");

                float newStrength = std::stof(args.back().value);

                lightsBuffer.lights[std::stoi(args.front().value)].strength = newStrength;

                Argument returnArgument;
                newStrength >> returnArgument;
                returnArgument.value.insert(0, "Strength set to ");

                return returnArgument;
            }
    ));
    //console.AddCommand(new CommandGetSet<float>("light_lightStrength", &worldModel->lightData.lightStrength));
    console.AddCommand(new CommandGetSet<float>("light_ambientStrength", &lightsBuffer.ambientStrength));
}

void LightManager::Update(Timer& deltaTimer, PrimitiveDrawer& primitiveDrawer)
{
    if(freezeLights)
    {
        if(drawLightSpheres)
            for(int i = 0; i < lightCount; ++i)
                primitiveDrawer.DrawSphere(lightsBuffer.lights[i].position, lightsBuffer.lights[i].strength, lightsBuffer.lights[i].color);

        return;
    }

    for(int i = 0; i < lightCount; ++i)
    {
        lightsBuffer.lights[i].lifetime += deltaTimer.GetDeltaMillisecondsFraction();

        lightsBuffer.lights[i].strength = GetLightRadius(lightsBuffer.lights[i].lifetime);

        if(lightsBuffer.lights[i].lifetime >= lightLifetime)
        {
            auto oldPosition = lightsBuffer.lights[i].position;
            lightsBuffer.lights[i] = GetNewLight();
            lightsBuffer.lights[i].position = oldPosition;

            if(lightPositionStrategy == CLUSTERED)
                lightsBuffer.lights[i].position += clusterPositions[i / (lightCount / lightClusters)];
        }

        if(drawLightSpheres)
            primitiveDrawer.DrawSphere(lightsBuffer.lights[i].position, lightsBuffer.lights[i].strength, lightsBuffer.lights[i].color);
    }
}

LightsBuffer& LightManager::GetLightsBuffer()
{
    return lightsBuffer;
}

LightData LightManager::GetNewLight()
{
    LightData light;

    light.position = GetRandomLightPosition();
    //light.color = glm::vec3(rand() / (float)RAND_MAX, rand() / (float)RAND_MAX, rand() / (float)RAND_MAX);
    light.color = glm::vec3(1.0f, 204.0f / 255.0f, 0.0f);
    light.lifetime = 0.0f;
    light.strength = GetLightRadius(0.0f);

    return light;
}

LightData LightManager::GetRandomLight()
{
    LightData light;

    light.position = GetRandomLightPosition();
    //light.color = glm::vec3(rand() / (float)RAND_MAX, rand() / (float)RAND_MAX, rand() / (float)RAND_MAX);
    light.color = glm::vec3(1.0f, 204.0f / 255.0f, 0.0f);
    light.lifetime = rand() / (float)RAND_MAX * lightLifetime;
    light.strength = GetLightRadius(light.lifetime);

    return light;
}

glm::vec3 LightManager::GetRandomLightPosition()
{
    if(lightsBuffer.lights.size() == 0)
        return glm::vec3(3.0f, 1.0f, 0.0f);
    else if(lightsBuffer.lights.size() == 1)
        return glm::vec3(0.0f, 1.0f, 0.0f);
    else
        return glm::vec3(-3.0f, 1.0f, 0.0f);

    glm::vec3 returnPosition;

    switch(lightPositionStrategy)
    {
        case RANDOM:
            returnPosition.x = ((rand() / (float)RAND_MAX) - 0.5f) * 2.0f * LIGHT_RANGE_X;
            returnPosition.y = (rand() / (float)RAND_MAX) * LIGHT_MAX_Y + 0.5f;
            returnPosition.z = ((rand() / (float)RAND_MAX) - 0.5f) * 2.0f * LIGHT_RANGE_Z;
            break;
        case CLUSTERED:
        {
            const static float maxDistSQ = lightClusterRadius * lightClusterRadius;
            float distanceSq = maxDistSQ + 1.0f;

            while(distanceSq > maxDistSQ)
            {
                returnPosition.x = ((rand() / (float)RAND_MAX) - 0.5f) * 2.0f * lightClusterRadius;
                returnPosition.y = ((rand() / (float)RAND_MAX) - 0.5f) * 2.0f * lightClusterRadius;
                returnPosition.z = ((rand() / (float)RAND_MAX) - 0.5f) * 2.0f * lightClusterRadius;

                distanceSq = glm::dot(returnPosition, returnPosition);
            }
            break;
        }
    }

    return returnPosition;
}

float LightManager::GetLightRadius(float lifetime)
{
    return std::sin((lifetime / lightLifetime) * glm::pi<float>()) * (lightMaxStrength - lightMinStrength) + lightMinStrength;
    //if(lifetime <= lightLifetime * 0.5f)
    //    return (lifetime / (lightLifetime * 0.5f)) * (lightMaxStrength - lightMinStrength) + lightMinStrength;
    //else
    //    return ((lightLifetime * 0.5f - (lifetime - lightLifetime * 0.5f)) / (lightLifetime * 0.5f)) * (lightMaxStrength - lightMinStrength) + lightMinStrength;
}