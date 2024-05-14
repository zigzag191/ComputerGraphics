#pragma once

#include "GLFW/glfw3.h"

#include "Camera.h"
#include "WindowUtil.h"

struct FPSCameraController
{
    static void UpdateFPSCamera(Camera& camera, GLFWwindow* window, float dt)
    {
        static CursorPosition prev = CursorPosition::Current(window);
        CursorPosition current = CursorPosition::Current(window);

        float diffPitch = prev.y - current.y;
        float diffYaw = current.x - prev.x;

        float pitch = diffPitch * 5.0f * dt;
        float yaw = diffYaw * 5.0f * dt;

        camera.yawDegrees += yaw;
        camera.pitchDegrees += pitch;
        if (camera.pitchDegrees > 89.0f)
        {
            camera.pitchDegrees = 89.0f;
        }
        if (camera.pitchDegrees < -89.0f)
        {
            camera.pitchDegrees = -89.0f;
        }

        prev = current;

        glm::vec3 moveVector{ 0 };
        if (GLFWKeyIsPressed(window, GLFW_KEY_W))
        {
            moveVector += camera.forwardDirection;
        }
        if (GLFWKeyIsPressed(window, GLFW_KEY_A))
        {
            moveVector -= camera.rightDirection;
        }
        if (GLFWKeyIsPressed(window, GLFW_KEY_S))
        {
            moveVector -= camera.forwardDirection;
        }
        if (GLFWKeyIsPressed(window, GLFW_KEY_D))
        {
            moveVector += camera.rightDirection;
        }
        if (GLFWKeyIsPressed(window, GLFW_KEY_E))
        {
            moveVector += Camera::WorldUpVector;
        }
        if (GLFWKeyIsPressed(window, GLFW_KEY_Q))
        {
            moveVector -= Camera::WorldUpVector;
        }

        if (!IsZeroLength(moveVector))
        {
            camera.position += glm::normalize(moveVector) * 5.0f * dt;
        }

        camera.SyncDirectionVectors();
    }

private:
    static bool IsZeroLength(const glm::vec3& v)
    {
        return v.x == 0.0f && v.y == 0.0f && v.z == 0.0f;
    }
};
