#pragma once

#include "glm/glm.hpp"

struct Frustum
{
	float nearPlane;
	float farPlane;
	float horizontalFovDegrees;
};

struct Camera
{
	static inline const glm::vec3 WorldUpVector{ 0.0f, 1.0f, 0.0f };

	glm::vec3 position;
	glm::vec3 forwardDirection;
	glm::vec3 rightDirection;
	glm::vec3 upDirection;
	Frustum frustum;
	float yawDegrees;
	float pitchDegrees;

    glm::mat4 BuildProjectionMatrix(int screenWidth, int screenHeight) const
    {
        glm::mat4 result{ 0.0f };

        // perspective
        /*const float fovTan = std::tan(glm::radians(frustum.horizontalFovDegrees / 2.0f));
        const float n = frustum.nearPlane;
        const float f = frustum.farPlane;
        const float r = n * fovTan;
        const float t = r * (static_cast<float>(screenHeight) / screenWidth);
        result[0][0] = n / r;
        result[1][1] = n / t;
        result[2][2] = -((f + n) / (f - n));
        result[3][2] = -2.0f * ((f * n) / (f - n));
        result[2][3] = -1.0f;*/

        const float n = frustum.nearPlane;
        const float f = frustum.farPlane;
        const float r = 4.0f;
        const float t = 4.0f * (static_cast<float>(screenHeight) / screenWidth);

        result[0][0] = 1.0f / r;
        result[1][1] = 1.0f / t;
        result[2][2] = -2.0f / (f - n);
        result[3][2] = -(f + n) / (f - n);
        result[3][3] = 1.0f;

        return result;
    }

    glm::mat4 BuildWorldToCameraMatrix() const
    {
        glm::mat4 inverseRotation{ 1.0f };
        inverseRotation[0] = glm::vec4(rightDirection, 0.0f);
        inverseRotation[1] = glm::vec4(upDirection, 0.0f);
        inverseRotation[2] = glm::vec4(-forwardDirection, 0.0f);

        inverseRotation = glm::transpose(inverseRotation);

        glm::mat4 inverseTransorm{ 1.0f };
        inverseTransorm[3] = glm::vec4(-position, 1.0f);

        return inverseRotation * inverseTransorm;
    }

    void SyncDirectionVectors()
    {
        const float cosPitch = std::cos(glm::radians(pitchDegrees));
        const float sinPitch = std::sin(glm::radians(pitchDegrees));
        const float cosYaw = std::cos(glm::radians(yawDegrees));
        const float sinYaw = std::sin(glm::radians(yawDegrees));

        forwardDirection.x = cosYaw * cosPitch;
        forwardDirection.y = sinPitch;
        forwardDirection.z = sinYaw * cosPitch;

        rightDirection = glm::normalize(glm::cross(Camera::WorldUpVector, -forwardDirection));
        upDirection = glm::normalize(glm::cross(-forwardDirection, rightDirection));
    }
};
