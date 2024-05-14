#pragma once

#include "GLFW/glfw3.h"

struct CursorPosition
{
    float x = 0.0f;
    float y = 0.0f;

    static CursorPosition Current(GLFWwindow* window)
    {
        double x, y;
        glfwGetCursorPos(window, &x, &y);
        return { static_cast<float>(x), static_cast<float>(y) };
    }
};

inline bool GLFWKeyIsPressed(GLFWwindow* window, int keyCode)
{
    return glfwGetKey(window, keyCode) == GLFW_PRESS;
}
