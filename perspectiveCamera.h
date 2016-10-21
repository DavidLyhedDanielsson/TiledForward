#ifndef perspectiveCamera_h__
#define perspectiveCamera_h__

#include <vector>

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

/**
* Free-flying quaternion-based camera
*
* Uses a reversed depth buffer
*/
class PerspectiveCamera
{
public:
	PerspectiveCamera();
	virtual ~PerspectiveCamera();

	/**
	* Initializes with the given horizontal field of view
	*
	* Example:\n
	*
	* \param position start at this position
	* \param lookAt look at this position at the start
	* \param fov horizontal field of view given in radians
	* \param aspectRatio
	* \param nearPlane
	* \param farPlane
	*/
	virtual void InitFovHorizontal(glm::vec3 position
                                   , glm::vec3 lookAt
                                   , float fov
								   , int width
                                   , int height
								   , float nearPlane
								   , float farPlane);
	/**
	* Initializes with the given vertical field of view
	*
	* Example:\n
	*
	* \param position start at this position
	* \param lookAt look at this position at the start
	* \param fov vertical field of view given in radians
	* \param aspectRatio
	* \param nearPlane
	* \param farPlane
	*/
	virtual void InitFovVertical(glm::vec3 position
								 , glm::vec3 lookAt
								 , float fov
                                 , int width
                                 , int height
								 , float nearPlane
								 , float farPlane);

	/**
	* Sets perspective related settings
	*
	* \param fov horizontal field of view given in radians
	* \param aspectRatio
	* \param nearPlane
	* \param farPlane
	*/
	virtual void SetPerspectiveHorizontal(float fov, int width, int height, float nearPlane, float farPlane);
	/**
	* Sets perspective related settings
	*
	* \param fov vertical field of view given in radians
	* \param aspectRatio
	* \param nearPlane
	* \param farPlane
	*/
	virtual void SetPerspectiveVertical(float fov, int width, int height, float nearPlane, float farPlane);

	float HorizontalFOVToVertical(float fov, int width, int height) const;
	float VerticalFOVToHorizontal(float fov, int width, int height) const;

	/**
	* Moves the camera forward
	*
	* \param units how far to move the camera
	*/
	void MoveFoward(float units);
	/**
	* Moves the camera to the right
	*
	* \param units how far to move the camera
	*/
	void MoveRight(float units);
	/**
	* Moves the camera up
	*
	* \param units how far to move the camera
	*/
	void MoveUp(float units);

	/**
	* Makes the camera look at \p position
	*
	* \param position
	*/
	virtual void LookAt(glm::vec3 position);
	/**
	* Makes the camera look towards \p direction
	*
	* \param direction
	*/
	virtual void LookTo(glm::vec3 direction);
	/**
	* Rotates the camera around its own axes (pitch/yaw)
	*
	* \param angle angle to rotate the camera given in radians
	*/
	virtual void Rotate(glm::vec2 angle);

	/**
	* Sets the camera's position
	*
	* \param position
	*/
	virtual void SetPosition(glm::vec3 position);

	/**
	* Gets the current position
	*/
	glm::vec3 GetPosition() const;
	/**
	* Gets the current projection matrix
	*/
	glm::mat4x4 GetProjectionMatrix() const;
	/**
	* Gets the view matrix
	*/
	glm::mat4x4 GetViewMatrix() const;

	/**
	* Gets the camera's right vector
	*/
	glm::vec3 GetRight() const;
	/**
	* Gets the camera's up vector
	*/
	glm::vec3 GetUp() const;
	/**
	* Gets the camera's forward vector
	*/
	glm::vec3 GetForward() const;
	/**
	* Gets the camera's rotation quaternion
	*/
	glm::quat GetRotationQuaternion() const;

	float GetNearPlane() const;
	float GetFarPlane() const;
	float GetPitch() const;
	float GetYaw() const;

	std::vector<glm::vec4> GetFrustumCorners() const;
protected:
	float aspectRatio;
	float fovVertical;

	float nearPlane;
	float farPlane;

	float pitch;
	float yaw;

	glm::vec3 position;
	glm::quat rotationQuaternion;
	glm::mat4x4 projectionMatrix;

	glm::quat CalcLookAtQuaternion(glm::vec3 position, glm::vec3 lookAt) const;
};


#endif // perspectiveCamera_h__
