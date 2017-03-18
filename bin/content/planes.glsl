const int PLANE_TOP = 0;
const int PLANE_BOTTOM = 1;
const int PLANE_LEFT = 2;
const int PLANE_RIGHT = 3;

vec4 CreatePlane(vec3 far0, vec3 far1)
{
    vec3 planeABC;
    planeABC = normalize(cross(far0, far1));
    float dist = 0.0f; //dot(far0, planeABC);

    return vec4(planeABC, dist);
}

const int BOTTOM_LEFT = 0;
const int BOTTOM_RIGHT = 1;
const int TOP_RIGHT = 2;
const int TOP_LEFT = 3;
const int CENTER = 4;

vec3[5] CreateFarPoints()
{
    const vec2 offsets[5] =
    {
        vec2(0.0f, 0.0f)
        , vec2(1.0f, 0.0f)
        , vec2(1.0f, 1.0f)
        , vec2(0.0f, 1.0f)
        , vec2(0.5f, 0.5f)
    };

    vec3 viewPositions[5];

    for(int j = 0; j < 5; ++j)
    {
        vec3 ndcPosition = vec3(((vec2(gl_WorkGroupID.xy) + offsets[j])
                                        * vec2(WORK_GROUP_SIZE_X, WORK_GROUP_SIZE_Y))
                                        / vec2(screenWidth, screenHeight), 1.0f);
        ndcPosition.x *= 2.0f;
        ndcPosition.x -= 1.0f;
        ndcPosition.y *= 2.0f;
        ndcPosition.y -= 1.0f;

        vec4 unprojectedPosition = projectionInverseMatrix * vec4(ndcPosition, 1.0f);
        unprojectedPosition /= unprojectedPosition.w;

        viewPositions[j] = vec3(unprojectedPosition);
    }

    return viewPositions;
}

vec3[5] CreateFarPoints(uvec2 workGroupSize)
{
    const vec2 offsets[5] =
    {
        vec2(0.0f, 0.0f)
        , vec2(1.0f, 0.0f)
        , vec2(1.0f, 1.0f)
        , vec2(0.0f, 1.0f)
        , vec2(0.5f, 0.5f)
    };

    vec3 viewPositions[5];

    for(int j = 0; j < 5; ++j)
    {
        vec3 ndcPosition = vec3(((vec2(gl_WorkGroupID.xy) + offsets[j])
                                        * vec2(workGroupSize.x, workGroupSize.y))
                                        / vec2(screenWidth, screenHeight), 1.0f);
        ndcPosition.x *= 2.0f;
        ndcPosition.x -= 1.0f;
        ndcPosition.y *= 2.0f;
        ndcPosition.y -= 1.0f;

        vec4 unprojectedPosition = projectionInverseMatrix * vec4(ndcPosition, 1.0f);
        unprojectedPosition /= unprojectedPosition.w;

        viewPositions[j] = vec3(unprojectedPosition);
    }

    return viewPositions;
}

vec4[4] CreatePlanes(vec3 viewPositions[5])
{
    vec4 planes[4];

    planes[PLANE_TOP] = CreatePlane(viewPositions[TOP_RIGHT], viewPositions[TOP_LEFT]);
    planes[PLANE_BOTTOM] = CreatePlane(viewPositions[BOTTOM_LEFT], viewPositions[BOTTOM_RIGHT]);
    planes[PLANE_RIGHT] = CreatePlane(viewPositions[BOTTOM_RIGHT], viewPositions[TOP_RIGHT]);
    planes[PLANE_LEFT] = CreatePlane(viewPositions[TOP_LEFT], viewPositions[BOTTOM_LEFT]);

    return planes;
}