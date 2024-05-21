#include <vector>
#include <queue>
#include <array>
#include <utility>
#include <algorithm>

#include "SFML/Graphics.hpp"
#include "glm/glm.hpp"
#include "glm/ext/matrix_transform.hpp"

#include "Bitmap.h"

using namespace std;
using namespace sf;
using namespace glm;

constexpr float FovDegrees = 60.0f;
mat4 PerspectiveProjectionMatrix{ 0.0f };
mat4 OrthographicProjectionMatrix{ 0.0f };
const Pixel DebugColor = { 255, 0, 0, 255 };

struct Light
{
    vec3 direction;
    vec4 color;
};

struct Triangle
{
    vec4 vertices[3];
    vec3 normal;
};

struct Model
{
    vector<Triangle> triangles;
    mat4 modelToWorldTransform;
    vec4 diffuseColor;

    void SetNormals()
    {
        for (auto& triangle : triangles)
        {
            auto ab = vec3{ triangle.vertices[1] - triangle.vertices[0] };
            auto ac = vec3{ triangle.vertices[2] - triangle.vertices[0] };
            triangle.normal = cross(ab, ac);
        }
    }
};

vec4 DivideByW(const vec4& vertex)
{
    return {
        vertex.x / vertex.w,
        vertex.y / vertex.w,
        vertex.z / vertex.w,
        vertex.w
    };
}

Point NdcToScreenSpace(const Bitmap& bitmap, const vec3& vertex)
{
    float sx = vertex.x + 1.0f;
    float sy = vertex.y - 1.0f;

    return {
        static_cast<int>(sx * 0.5f * bitmap.width),
        static_cast<int>(sy * -0.5f * bitmap.height)
    };
}

void DrawModel(Bitmap& bitmap, const Model& model, const vector<Light>& lights, const mat4& projectionMatrix, bool showWireframe = false)
{
    vector<Triangle> visibleTriangles;

    for (auto& triangle : model.triangles)
    {
        auto worldSpaceTriangle = Triangle{
            model.modelToWorldTransform * triangle.vertices[0],
            model.modelToWorldTransform * triangle.vertices[1],
            model.modelToWorldTransform * triangle.vertices[2]
        };
        worldSpaceTriangle.normal = transpose(inverse(mat3{ model.modelToWorldTransform })) * triangle.normal;

        if (dot(worldSpaceTriangle.normal, vec3{ worldSpaceTriangle.vertices[0] }) >= 0.0f)
        {
            //continue;
        }

        auto& cameraSpaceTriangle = worldSpaceTriangle;

        auto ndcSpaceTriangle = Triangle{
            DivideByW(projectionMatrix * cameraSpaceTriangle.vertices[0]),
            DivideByW(projectionMatrix * cameraSpaceTriangle.vertices[1]),
            DivideByW(projectionMatrix * cameraSpaceTriangle.vertices[2])
        };
        ndcSpaceTriangle.normal = cameraSpaceTriangle.normal;

        visibleTriangles.push_back(ndcSpaceTriangle);
    }

    std::sort(visibleTriangles.begin(), visibleTriangles.end(), [](const auto& t1, const auto& t2) {
        float z1 = (t1.vertices[0].z + t1.vertices[1].z + t1.vertices[2].z) / 3.0f;
        float z2 = (t2.vertices[0].z + t2.vertices[1].z + t2.vertices[2].z) / 3.0f;
        return z1 > z2;
    });

    for (auto& triangle : visibleTriangles)
    {
        auto p1 = NdcToScreenSpace(bitmap, triangle.vertices[0]);
        auto p2 = NdcToScreenSpace(bitmap, triangle.vertices[1]);
        auto p3 = NdcToScreenSpace(bitmap, triangle.vertices[2]);

        auto color = vec4{ 0.0f, 0.0f, 0.0f, 0.0f };
        float ambientIntensity = 0.15f;
        float directLightIntensity = 1.0f - ambientIntensity;
        float perLightIntensity = lights.empty() ? 0.0f : directLightIntensity / lights.size();

        color += model.diffuseColor * ambientIntensity;

        for (auto& light : lights)
        {
            float cosAngIncidence = dot(normalize(triangle.normal), -normalize(light.direction));
            if (cosAngIncidence < 0.0f)
            {
                cosAngIncidence = 0.0f;
            }
            else if (cosAngIncidence > 1.0f)
            {
                cosAngIncidence = 1.0f;
            }
            color += cosAngIncidence * perLightIntensity * light.color;
        }

        auto pixel = Pixel{
            static_cast<uint8_t>(255 * color.x),
            static_cast<uint8_t>(255 * color.y),
            static_cast<uint8_t>(255 * color.z),
            255
        };

        bitmap.FillTriangle(p1, p2, p3, pixel);

        if (showWireframe)
        {
            bitmap.DrawTriangle(p1, p2, p3, DebugColor);
        }
    }
}

void BuildPrejectionMatrix(const Bitmap& bitmap)
{
    float r = tan(radians(FovDegrees / 2.0f));
    float t = r * (bitmap.height / static_cast<float>(bitmap.width));
    PerspectiveProjectionMatrix[0][0] = 1.0f / r;
    PerspectiveProjectionMatrix[1][1] = 1.0f / t;
    PerspectiveProjectionMatrix[2][2] = -1.0f;
    PerspectiveProjectionMatrix[2][3] = -1.0f;
    PerspectiveProjectionMatrix[3][2] = -2.0f;

    float ro = 2.0f;
    float to = 2.0f * (bitmap.height / static_cast<float>(bitmap.width));
    OrthographicProjectionMatrix[0][0] = 1.0f / ro;
    OrthographicProjectionMatrix[1][1] = 1.0f / to;
    OrthographicProjectionMatrix[2][2] = -2.0f / (1000.0f - 1.0f);
    OrthographicProjectionMatrix[3][2] = -(1000.0f + 1.0f) / (1000.0f - 1.0f);
    OrthographicProjectionMatrix[3][3] = 1.0f;
}

void UpdateTextureFromBitmap(Texture& texture, const Bitmap& bitmap)
{
    texture.update(reinterpret_cast<const Uint8*>(bitmap.pixels.data()));
}

Point GetBitmapCursorPostion(const Window& window, float screenPixelToBitmapPixelRatio)
{
    Vector2i cursorPosition = Mouse::getPosition(window);
    return {
        static_cast<int>(cursorPosition.x / screenPixelToBitmapPixelRatio),
        static_cast<int>(cursorPosition.y / screenPixelToBitmapPixelRatio)
    };
}

Pixel ColorToPixel(const Color& color)
{
    return { color.r, color.g, color.b, 255 };
}

Model GenerateCylinder(int steps, float height, float radius)
{
    height /= 2.0f;

    vector<vec3> circle;

    float angleStep = 360.0f / steps;
    for (float angle = 0.0f; angle < 360.0f; angle += angleStep)
    {
        float r = radians(angle);
        circle.push_back({ radius * cos(r), 0.0f, radius * sin(r) });
    }

    Model result;

    for (int i = 1; i < circle.size() - 1; ++i)
    {
        auto& p1 = circle[0];
        auto& p2 = circle[i];
        auto& p3 = circle[i + 1];

        result.triangles.push_back({{
            {  p1.x, height, p1.z, 1.0f },
            {  p3.x, height, p3.z, 1.0f },
            {  p2.x, height, p2.z, 1.0f }
        }});

        result.triangles.push_back({{
            {  p3.x, -height, p3.z, 1.0f },
            {  p1.x, -height, p1.z, 1.0f },
            {  p2.x, -height, p2.z, 1.0f }
        }});
    }

    auto addEdgeTriangle = [&](int i, int j)
    {
        result.triangles.push_back({{
            { circle[i].x, -height, circle[i].z, 1.0f },
            { circle[i].x,  height, circle[i].z, 1.0f },
            { circle[j].x,  height, circle[j].z, 1.0f }
        }});

        result.triangles.push_back({{
            { circle[i].x, -height, circle[i].z, 1.0f },
            { circle[j].x,  height, circle[j].z, 1.0f },
            { circle[j].x, -height, circle[j].z, 1.0f }
        }});
    };

    for (int i = 0; i < circle.size() - 1; ++i)
    {
        addEdgeTriangle(i, i + 1);
    }
    addEdgeTriangle(static_cast<int>(circle.size() - 1), 0);

    return result;
}

int main()
{
    RenderWindow window(VideoMode(800, 600), "Software renderer");

    Vector2f windowSize = window.getView().getSize();
    float screenPixelToBitmapPixelRatio = 5;
    Bitmap bitmap = Bitmap::New(
        static_cast<int>(windowSize.x / screenPixelToBitmapPixelRatio),
        static_cast<int>(windowSize.y / screenPixelToBitmapPixelRatio)
    );

    Texture texture;
    texture.create(bitmap.width, bitmap.height);

    RectangleShape screen;
    screen.setSize(windowSize);
    screen.setTexture(&texture, false);

    BuildPrejectionMatrix(bitmap);

    Model model = GenerateCylinder(13, 2, 1);
    model.modelToWorldTransform = mat4{ 1.0f };
    model.modelToWorldTransform[3] = { 0.0f, 0.0f, -4.0f, 1.0f };
    model.diffuseColor = { 1.0f, 1.0f, 1.0f, 1.0f };
    model.SetNormals();

    Clock clock;
    float rotationSpeed = 30.0f;

    vector<Light> lights;
    lights.push_back({ { 1.0f, -0.25f, -1.0f }, { 1.0f, 0.0f, 0.0f, 1.0f } });
    lights.push_back({ { -1.0f, -0.25f, -1.0f }, { 0.0f, 0.0f, 1.0f, 1.0f } });
    lights.push_back({ { 0.0f, 0.25f, -1.0f }, { 0.0f, 1.0f, 0.0f, 1.0f } });

    bool useOrtho = false; bool pWasPressed = false;
    bool drawWireframe = false; bool wWasPressed = false;

    while (window.isOpen())
    {
        Event event;
        while (window.pollEvent(event))
        {
            if (event.type == Event::Closed)
            {
                window.close();
            }
        }

        float dt = clock.getElapsedTime().asSeconds();
        clock.restart();

        if (Keyboard::isKeyPressed(Keyboard::Y))
        {
            model.modelToWorldTransform = rotate(model.modelToWorldTransform, radians(rotationSpeed * dt), vec3{ 0.0f, 1.0f, 0.0f });
        }
        if (Keyboard::isKeyPressed(Keyboard::X))
        {
            model.modelToWorldTransform = rotate(model.modelToWorldTransform, radians(rotationSpeed * dt), vec3{ 1.0f, 0.0f, 0.0f });
        }
        if (Keyboard::isKeyPressed(Keyboard::Z))
        {
            model.modelToWorldTransform = rotate(model.modelToWorldTransform, radians(rotationSpeed * dt), vec3{ 0.0f, 0.0f, 1.0f });
        }

        bool pIsPressed = Keyboard::isKeyPressed(Keyboard::P);
        if (!pWasPressed && pIsPressed)
        {
            useOrtho = !useOrtho;
            pWasPressed = true;
        }
        else if (pWasPressed && !pIsPressed)
        {
            pWasPressed = false;
        }

        bool wIsPressed = Keyboard::isKeyPressed(Keyboard::W);
        if (!wWasPressed && wIsPressed)
        {
            drawWireframe = !drawWireframe;
            wWasPressed = true;
        }
        else if (wWasPressed && !wIsPressed)
        {
            wWasPressed = false;
        }

        bitmap.Clear();
        window.clear();

        DrawModel(bitmap, model, lights, useOrtho ? OrthographicProjectionMatrix : PerspectiveProjectionMatrix, drawWireframe);

        UpdateTextureFromBitmap(texture, bitmap);
        window.draw(screen);
        window.display();
    }

    return 0;
}
