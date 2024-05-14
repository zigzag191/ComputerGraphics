#pragma once

#include "GLFW/glfw3.h"

struct Clock
{
    float currentTime = CurrentTime();

    static float CurrentTime()
    {
        return static_cast<float>(glfwGetTime());
    }

    float GetElapsedTime()
    {
        float time = CurrentTime();
        float dt = time - currentTime;
        currentTime = time;
        return dt;
    }
};
