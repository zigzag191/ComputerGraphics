#include <vector>
#include <queue>
#include <array>
#include <utility>

#include "SFML/Graphics.hpp"
#include "glm/glm.hpp"
#include "glm/ext/matrix_transform.hpp"

#include "Bitmap.h"

using namespace std;
using namespace sf;
using namespace glm;

constexpr float FovDegrees = 60.0f;
mat4 ProjectionMatrix{ 0.0f };
const Pixel BaseColor = { 0, 0, 255, 255 };
const Pixel DebugColor = { 255, 0, 0, 255 };

struct Triangle
{
    vec4 vertices[3];
    vec3 normal;
};

struct Model
{
    vector<Triangle> triangles;
    mat4 modelToWorldTransform;

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

void DrawModel(Bitmap& bitmap, const Model& model)
{
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
            continue;
        }

        auto& cameraSpaceTriangle = worldSpaceTriangle;

        auto ndcSpaceTriangle = Triangle{
            DivideByW(ProjectionMatrix * cameraSpaceTriangle.vertices[0]),
            DivideByW(ProjectionMatrix * cameraSpaceTriangle.vertices[1]),
            DivideByW(ProjectionMatrix * cameraSpaceTriangle.vertices[2])
        };

        auto p1 = NdcToScreenSpace(bitmap, ndcSpaceTriangle.vertices[0]);
        auto p2 = NdcToScreenSpace(bitmap, ndcSpaceTriangle.vertices[1]);
        auto p3 = NdcToScreenSpace(bitmap, ndcSpaceTriangle.vertices[2]);

        bitmap.FillTriangle(p1, p2, p3, BaseColor);
        bitmap.DrawTriangle(p1, p2, p3, DebugColor);
    }
}

void BuildPrejectionMatrix(mat4& matrix, const Bitmap& bitmap)
{
    float r = tan(radians(FovDegrees / 2.0f));
    float t = r * (bitmap.height / static_cast<float>(bitmap.width));

    matrix[0][0] = 1.0f / r;
    matrix[1][1] = 1.0f / t;
    matrix[2][2] = -1.0f;
    matrix[2][3] = -1.0f;
    matrix[3][2] = -2.0f;
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

Model GenerateCylinder()
{
    int steps = 15;
    vector<vec3> circle;

    float angleStep = 360.0f / steps;
    for (float angle = 0.0f; angle < 360.0f; angle += angleStep)
    {
        float r = radians(angle);
        circle.push_back({ cos(r), 0.0f, sin(r) });
    }

    Model result;

    for (int i = 1; i < circle.size() - 1; ++i)
    {
        auto& p1 = circle[0];
        auto& p2 = circle[i];
        auto& p3 = circle[i + 1];

        result.triangles.push_back({{
            {  p1.x, 1.0f, p1.z, 1.0f },
            {  p3.x, 1.0f, p3.z, 1.0f },
            {  p2.x, 1.0f, p2.z, 1.0f }
        }});

        result.triangles.push_back({{
            {  p3.x, -1.0f, p3.z, 1.0f },
            {  p1.x, -1.0f, p1.z, 1.0f },
            {  p2.x, -1.0f, p2.z, 1.0f }
        }});
    }

    auto addEdgeTriangle = [&](int i, int j)
    {
        result.triangles.push_back({{
            { circle[i].x, -1.0f, circle[i].z, 1.0f },
            { circle[i].x, 1.0f, circle[i].z, 1.0f },
            { circle[j].x, 1.0f, circle[j].z, 1.0f }
        }});

        result.triangles.push_back({{
            { circle[i].x, -1.0f, circle[i].z, 1.0f },
            { circle[j].x, 1.0f, circle[j].z, 1.0f },
            { circle[j].x, -1.0f, circle[j].z, 1.0f }
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
    float screenPixelToBitmapPixelRatio = 3;
    Bitmap bitmap = Bitmap::New(
        static_cast<int>(windowSize.x / screenPixelToBitmapPixelRatio),
        static_cast<int>(windowSize.y / screenPixelToBitmapPixelRatio)
    );

    Texture texture;
    texture.create(bitmap.width, bitmap.height);

    RectangleShape screen;
    screen.setSize(windowSize);
    screen.setTexture(&texture, false);

    BuildPrejectionMatrix(ProjectionMatrix, bitmap);

    Model model = GenerateCylinder();
    model.modelToWorldTransform = mat4{ 1.0f };
    model.modelToWorldTransform[3] = { 0.0f, 0.0f, -5.0f, 1.0f };
    model.SetNormals();

    Clock clock;
    float rotationSpeed = 15.0f;

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

        bitmap.Clear();
        window.clear();

        DrawModel(bitmap, model);

        UpdateTextureFromBitmap(texture, bitmap);
        window.draw(screen);
        window.display();
    }

    return 0;
}
