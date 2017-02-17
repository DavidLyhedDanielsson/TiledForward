#include "perspectiveCamera.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/quaternion.hpp>

#include <cmath>

PerspectiveCamera::PerspectiveCamera()
	: aspectRatio(0.0f)
	, fovVertical(0.0f)
	, nearPlane(0.0f)
	, farPlane(0.0f)
	, pitch(0.0f)
	, yaw(0.0f)
	, position(0.0f, 0.0f, 0.0f)
	, rotationQuaternion(0.0f, 0.0f, 0.0f, 0.0f)
	, projectionMatrix(1.0f, 0.0f, 0.0f, 0.0f
						, 0.0f, 1.0f, 0.0f, 0.0f
						, 0.0f, 0.0f, 1.0f, 0.0f
						, 0.0f, 0.0f, 0.0f, 1.0f)
{ }

PerspectiveCamera::~PerspectiveCamera()
{ }

void PerspectiveCamera::InitFovHorizontal(glm::vec3 position
                                          , glm::vec3 lookAt
                                          , float fov
                                          , int width
                                          , int height
                                          , float nearPlane
                                          , float farPlane)
{
	InitFovVertical(position, lookAt, HorizontalFOVToVertical(fov, width, height), width, height, nearPlane, farPlane);
}

void PerspectiveCamera::InitFovVertical(glm::vec3 position
                                        , glm::vec3 lookAt
                                        , float fov
                                        , int width
                                        , int height
                                        , float nearPlane
                                        , float farPlane)
{
	SetPerspectiveVertical(fov, width, height, nearPlane, farPlane);
	SetPosition(position);
	LookAt(lookAt);
}

void PerspectiveCamera::SetPerspectiveHorizontal(float fov, int width, int height, float nearPlane, float farPlane)
{
	SetPerspectiveVertical(HorizontalFOVToVertical(fov, width, height), width, height, nearPlane, farPlane);
}

void PerspectiveCamera::SetPerspectiveVertical(float fov, int width, int height, float nearPlane, float farPlane)
{
	this->fovVertical = fov;
	this->aspectRatio = aspectRatio;
	this->nearPlane = farPlane;
	this->farPlane = nearPlane;

    projectionMatrix = glm::perspectiveFovLH(fovVertical, (float)width, (float)height, nearPlane, farPlane);
}

float PerspectiveCamera::HorizontalFOVToVertical(float fov, int width, int height) const
{
	return static_cast<float>(2.0 * atan(tan(fov * 0.5) / ((float)width / height)));
}

float PerspectiveCamera::VerticalFOVToHorizontal(float fov, int width, int height) const
{
	return static_cast<float>(2.0 * atan(((float)width / height) * tan(fov * 0.5)));
}

void PerspectiveCamera::MoveFoward(float units)
{
    position = GetForward() * units + position;
}

void PerspectiveCamera::MoveRight(float units)
{
    position = GetRight() * units + position;
}

void PerspectiveCamera::MoveUp(float units)
{
    position = GetUp() * units + position;
}

void PerspectiveCamera::LookAt(glm::vec3 position)
{
	rotationQuaternion = CalcLookAtQuaternion(this->position, position);

	yaw = glm::yaw(rotationQuaternion);
	pitch = glm::pitch(rotationQuaternion);
}

void PerspectiveCamera::LookTo(glm::vec3 direction)
{
	rotationQuaternion = CalcLookAtQuaternion(position, position + direction);

    yaw = glm::yaw(rotationQuaternion);
    pitch = glm::pitch(rotationQuaternion);
}

void PerspectiveCamera::Rotate(glm::vec2 angle)
{
	if(pitch + angle.y >= glm::half_pi<float>())
	{
		angle.y = glm::half_pi<float>() - pitch;
		pitch = glm::half_pi<float>();
	}
	else if(pitch + angle.y <= -glm::half_pi<float>())
	{
		angle.y = -glm::half_pi<float>() - pitch;
		pitch = -glm::half_pi<float>();
	}
	else
		pitch += angle.y;

	yaw += angle.x;
	yaw = std::fmod(glm::two_pi<float>() + std::fmod(yaw, glm::two_pi<float>()), glm::two_pi<float>());

	glm::vec3 right = { 1.0f, 0.0f, 0.0f };
    glm::vec3 up = { 0.0f, 1.0f, 0.0f };

    rotationQuaternion = glm::angleAxis(yaw, up) * glm::angleAxis(pitch, right);

    //DirectX::XMStoreFloat4(&rotationQuaternion, DirectX::XMQuaternionMultiply(DirectX::XMQuaternionRotationAxis(xmRight, pitch), DirectX::XMQuaternionRotationAxis(xmUp, yaw)));
}

void PerspectiveCamera::SetPosition(glm::vec3 position)
{
	this->position = position;
}

glm::vec3 PerspectiveCamera::GetPosition() const
{
	return position;
}

glm::mat4x4 PerspectiveCamera::GetProjectionMatrix() const
{
	return projectionMatrix;
}


glm::mat4x4 PerspectiveCamera::GetViewMatrix() const
{
//	auto viewMatrix = DirectX::XMStoreFloat4x4(DirectX::XMMatrixRotationQuaternion(DirectX::XMQuaternionInverse(DirectX::XMLoadFloat4(&rotationQuaternion))));
//
//	auto xmRight = DirectX::XMLoadFloat3(viewMatrix(0, 0), viewMatrix(1, 0), viewMatrix(2, 0));
//	auto xmUp = DirectX::XMLoadFloat3(viewMatrix(0, 1), viewMatrix(1, 1), viewMatrix(2, 1));
//	auto xmForward = DirectX::XMLoadFloat3(viewMatrix(0, 2), viewMatrix(1, 2), viewMatrix(2, 2));
//
//	auto xmPosition = DirectX::XMLoadFloat3(&position);
//
//	float x = -DirectX::XMVectorGetX(DirectX::XMVector3Dot(xmPosition, xmRight));
//	float y = -DirectX::XMVectorGetY(DirectX::XMVector3Dot(xmPosition, xmUp));
//	float z = -DirectX::XMVectorGetZ(DirectX::XMVector3Dot(xmPosition, xmForward));
//
//	viewMatrix(3, 0) = x;
//	viewMatrix(3, 1) = y;
//	viewMatrix(3, 2) = z;
//
//	return viewMatrix;

    auto viewMatrix = glm::mat4_cast(glm::inverse(rotationQuaternion));

    glm::vec3 xmRight = { viewMatrix[0][0], viewMatrix[1][0], viewMatrix[2][0] };
    glm::vec3 xmUp = { viewMatrix[0][1], viewMatrix[1][1], viewMatrix[2][1] };
    glm::vec3 xmForward = { viewMatrix[0][2], viewMatrix[1][2], viewMatrix[2][2] };

    viewMatrix[3][0] = -glm::dot(position, xmRight);
    viewMatrix[3][1] = -glm::dot(position, xmUp);
    viewMatrix[3][2] = -glm::dot(position, xmForward);

    return viewMatrix;
}

glm::vec3 PerspectiveCamera::GetRight() const
{
    return rotationQuaternion * glm::vec3(1.0f, 0.0f, 0.0f);
}

glm::vec3 PerspectiveCamera::GetUp() const
{
    return rotationQuaternion * glm::vec3(0.0f, 1.0f, 0.0f);
}

glm::vec3 PerspectiveCamera::GetForward() const
{
    return rotationQuaternion * glm::vec3(0.0f, 0.0f, 1.0f);
}

glm::quat PerspectiveCamera::GetRotationQuaternion() const
{
	return rotationQuaternion;
}

glm::quat PerspectiveCamera::CalcLookAtQuaternion(glm::vec3 position, glm::vec3 lookAt) const
{
    glm::vec3 forward = glm::normalize(lookAt - position);
	glm::vec3 up = { 0.0f, 1.0f, 0.0f };

	auto right = glm::normalize(glm::cross(up, forward));
	up = glm::normalize(glm::cross(forward, right));

	glm::mat4x4 viewMatrix;

	viewMatrix[0][0] = right.x;
	viewMatrix[1][0] = right.y;
	viewMatrix[2][0] = right.z;
	viewMatrix[3][0] = 0.0f;

	viewMatrix[0][1] = up.x;
	viewMatrix[1][1] = up.y;
	viewMatrix[2][1] = up.z;
	viewMatrix[3][1] = 0.0f;

	viewMatrix[0][2] = forward.x;
	viewMatrix[1][2] = forward.y;
	viewMatrix[2][2] = forward.z;
	viewMatrix[3][2] = 0.0f;

	viewMatrix[0][3] = 0.0f;
	viewMatrix[1][3] = 0.0f;
	viewMatrix[2][3] = 0.0f;
	viewMatrix[3][3] = 1.0f;

	auto quaternion = glm::toQuat(viewMatrix);
	auto quaternionInverse = glm::inverse(glm::normalize(quaternion));

	return quaternionInverse;
}

float PerspectiveCamera::GetNearPlane() const
{
	return farPlane;
}

float PerspectiveCamera::GetFarPlane() const
{
	return nearPlane;
}

float PerspectiveCamera::GetPitch() const
{
	return pitch;
}

float PerspectiveCamera::GetYaw() const
{
	return yaw;
}

std::vector<glm::vec4> PerspectiveCamera::GetFrustumCorners() const
{
	std::vector<glm::vec4> corners;

	auto viewMatrix = GetViewMatrix();
	auto projectionMatrix = GetProjectionMatrix();

	glm::mat4x4 viewProjectionMatrixInverse = glm::inverse(viewMatrix * projectionMatrix);

	glm::vec4 ndcPositions[] =
	{
		{ -1.0f, -1.0f, -1.0f, 1.0f }
		,{ -1.0f, 1.0f, -1.0f , 1.0f}
		,{ 1.0f, 1.0f, -1.0f, 1.0f }
		,{ 1.0f, -1.0f, -1.0f, 1.0f }

		,{ -1.0f, -1.0f, 1.0f, 1.0f }
		,{ -1.0f, 1.0f, 1.0f, 1.0f }
		,{ 1.0f, 1.0f, 1.0f, 1.0f }
		,{ 1.0f, -1.0f, 1.0f, 1.0f }
	};

	for(int i = 0; i < 8; ++i)
		corners.push_back(ndcPositions[i] * viewProjectionMatrixInverse);

	return corners;
}

float PerspectiveCamera::GetFOVVertical() const
{
	return fovVertical;
}
