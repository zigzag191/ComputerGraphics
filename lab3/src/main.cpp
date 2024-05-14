#include <cstddef>

#include "vector"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "GL/glew.h"
#include "GLFW/glfw3.h"
#include "spdlog/spdlog.h"

#include "Camera.h"
#include "Clock.h"
#include "WindowUtil.h"
#include "FPSCameraController.h"
#include "Debug.h"
#include "Shader.h"

struct Vertex
{
    float x, y, z;
    float nx, ny, nz;
    float tx, ty;
};

struct Mesh
{
    GLuint vao;
    GLuint vbo;
    GLuint ibo;
    int indicesSize;
};

struct ModelInfo
{
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;
};

Mesh CreateMesh(const ModelInfo& modelInfo)
{
    Mesh result;
    result.indicesSize = static_cast<int>(modelInfo.indices.size());

    glCreateVertexArrays(1, &result.vao);
    glBindVertexArray(result.vao);

    glCreateBuffers(1, &result.vbo);
    glBindBuffer(GL_ARRAY_BUFFER, result.vbo);
    glBufferData(GL_ARRAY_BUFFER, modelInfo.vertices.size() * sizeof(modelInfo.vertices[0]), modelInfo.vertices.data(), GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), 0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(sizeof(float) * 3));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(sizeof(float) * 6));

    glCreateBuffers(1, &result.ibo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, result.ibo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, modelInfo.indices.size() * sizeof(unsigned int), modelInfo.indices.data(), GL_STATIC_DRAW);

    return result;
}

ModelInfo GeneratePyramid(int height, int radius, int edges)
{
    ModelInfo info;
    /*info.vertices.push_back({  0.0f,  1.0f, 3.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f });
    info.vertices.push_back({ -1.0f, -1.0f, 3.0f, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f });
    info.vertices.push_back({  1.0f, -1.0f, 3.0f, 0.0f, 0.0f, -1.0f, 1.0f, 1.0f });
    info.indices = { 0, 1, 2 };*/

    std::vector<glm::vec3> circle;

    float angleStep = 360.0f / edges;
    for (float angle = 0.0f; angle < 360.0f; angle += angleStep)
    {
        float r = glm::radians(angle);
        circle.push_back({ radius * cos(r), 0.0f, radius * sin(r) });
    }

    for (int i = 1; i < circle.size() - 1; ++i)
    {
        const auto& p1 = circle[0];
        const auto& p2 = circle[i];
        const auto& p3 = circle[i + 1];

        const auto n = glm::cross(p2 - p1, p3 - p1);

        int current = static_cast<int>(info.vertices.size());

        info.vertices.push_back({ p1.x, -height / 2.0f, p1.z, n.x, n.y, n.z, 0.0f, 0.0f });
        info.vertices.push_back({ p3.x, -height / 2.0f, p3.z, n.x, n.y, n.z, 1.0f, 1.0f });
        info.vertices.push_back({ p2.x, -height / 2.0f, p2.z, n.x, n.y, n.z, 1.0f, 0.0f });

        info.indices.push_back(current);
        info.indices.push_back(current + 1);
        info.indices.push_back(current + 2);
    }

    auto top = glm::vec3{ 0.0f, height / 2.0f, 0.0f };

    auto addEdge = [&](int i, int j)
    {
        const auto& p2 = circle[i];
        const auto& p3 = circle[j];

        const auto n = glm::cross(p3 - top, p2 - top);

        int current = static_cast<int>(info.vertices.size());

        info.vertices.push_back({ top.x, top.y, top.z, n.x, n.y, n.z, 0.0f, 0.0f });
        info.vertices.push_back({ p2.x, -height / 2.0f, p2.z, n.x, n.y, n.z, 1.0f, 0.0f });
        info.vertices.push_back({ p3.x, -height / 2.0f, p3.z, n.x, n.y, n.z, 1.0f, 1.0f });

        info.indices.push_back(current);
        info.indices.push_back(current + 1);
        info.indices.push_back(current + 2);
    };

    for (int i = 0; i < circle.size() - 1; ++i)
    {
        addEdge(i, i + 1);
    }
    addEdge(static_cast<int>(circle.size() - 1), 0);

    return info;
}

void DrawMesh(const Mesh& mesh)
{
    glBindVertexArray(mesh.vao);
    glDrawElements(GL_TRIANGLES, mesh.indicesSize, GL_UNSIGNED_INT, nullptr);
}

GLuint LoadTexture(std::string_view path)
{
    int width, height, bpp;
    stbi_set_flip_vertically_on_load(true);
    unsigned char* data = stbi_load(path.data(), &width, &height, &bpp, 0);

    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
    glBindTexture(GL_TEXTURE_2D, 0);

    stbi_image_free(data);

    return texture;
}

GLuint CreateSampler()
{
    GLuint sampler;
    glGenSamplers(1, &sampler);
    glSamplerParameteri(sampler, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glSamplerParameteri(sampler, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glSamplerParameteri(sampler, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glSamplerParameteri(sampler, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glSamplerParameteri(sampler, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    return sampler;
}

int main()
{
    spdlog::set_pattern("[%^%l%$] %v");

    if (!glfwInit())
    {
        spdlog::critical("Unable to init GLFW");
        return EXIT_FAILURE;
    }

    int screenWidth = 1000;
    int screeHeight = 800;
    auto window = glfwCreateWindow(screenWidth, screeHeight, "OpenGLDemo", nullptr, nullptr);
    if (window == nullptr)
    {
        spdlog::critical("Unable to create window");
        return EXIT_FAILURE;
    }

    glfwMakeContextCurrent(window);

    if (glewInit() != GLEW_OK)
    {
        spdlog::critical("Unable to init GLEW");
        return EXIT_FAILURE;
    }

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, true);
    glEnable(GL_DEBUG_OUTPUT);
    glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
    glDebugMessageCallback(GLDebugMessageCallback, nullptr);
    glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);

    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CW);

    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    auto modelInfo = GeneratePyramid(5, 3, 10);
    auto mesh = CreateMesh(modelInfo);

    auto shader = Shader::LoadProgram("res/simple.vert", "res/simple.frag");
    glUseProgram(shader);

    auto texture = LoadTexture("res/water.png");
    auto sampler = CreateSampler();
    glBindSampler(0, sampler);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture);
    Shader::SetTextureUniform(shader, "colorTexture", 0);

    Clock clock;
    Camera camera;
    camera.frustum.nearPlane = 0.1f;
    camera.frustum.farPlane = 1000.0f;
    camera.frustum.horizontalFovDegrees = 90.0f;
    camera.position = glm::vec3{ 0.0f, 0.0f, -6.0f };
    camera.pitchDegrees = 0.0f;
    camera.yawDegrees = 90.0f;
    camera.SyncDirectionVectors();

    float lightAngle = 0.0f;

    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();
        glClear(GL_COLOR_BUFFER_BIT);

        auto dt = clock.GetElapsedTime();

        if (GLFWKeyIsPressed(window, GLFW_KEY_ESCAPE))
        {
            glfwSetWindowShouldClose(window, true);
        }
        if (GLFWKeyIsPressed(window, GLFW_KEY_R))
        {
            lightAngle += 3.0f * dt;
        }

        FPSCameraController::UpdateFPSCamera(camera, window, dt);

        auto worldToCameraMatrix = camera.BuildWorldToCameraMatrix();
        auto projectionMatrix = camera.BuildProjectionMatrix(screenWidth, screeHeight);

        Shader::SetMat4Uniform(shader, "worldToCameraMatrix", worldToCameraMatrix);
        Shader::SetMat4Uniform(shader, "projectionMatrix", projectionMatrix);
        Shader::SetVec3Uniform(shader, "lightDirection", camera.forwardDirection);

        DrawMesh(mesh);

        glfwSwapBuffers(window);
    }

    glDeleteSamplers(1, &sampler);
    glDeleteTextures(1, &texture);
    glDeleteProgram(shader);
    glfwDestroyWindow(window);
    glfwTerminate();

	return EXIT_SUCCESS;
}
