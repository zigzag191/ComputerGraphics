#pragma once

#include <cstdint>
#include <vector>

struct Point
{
    int x, y;
};

struct Pixel
{
    std::uint8_t r, g, b, a;
};

struct Bitmap
{
    std::vector<Pixel> pixels;
    int width;
    int height;

    static Bitmap New(int pixelWidth, int pixelHeight)
    {
        Bitmap result;
        result.width = pixelWidth;
        result.height = pixelHeight;
        result.pixels.assign(pixelWidth * pixelHeight, { 0, 0, 0, 255 });
        return result;
    }

    bool Contains(const Point& p) const
    {
        int x = p.x;
        int y = p.y;
        return x >= 0 && y >= 0 && x < width && y < height;
    }

    void Clear()
    {
        for (auto& p : pixels)
        {
            p = { 0, 0, 0, 255 };
        }
    }

    bool DrawPixel(const Point& p, const Pixel& pixel)
    {
        int x = p.x;
        int y = p.y;
        if (!Contains(p))
        {
            return false;
        }
        pixels[y * width + x] = pixel;
        return true;
    }

    void DrawLine(const Point& p1, const Point& p2, const Pixel& pixel)
    {
        if (std::abs(p2.y - p1.y) < std::abs(p2.x - p1.x))
        {
            if (p1.x > p2.x)
            {
                DrawLineLow(p2, p1, pixel);
            }
            else
            {
                DrawLineLow(p1, p2, pixel);
            }
        }
        else
        {
            if (p1.y > p2.y)
            {
                DrawLineHigh(p2, p1, pixel);
            }
            else
            {
                DrawLineHigh(p1, p2, pixel);
            }
        }
    }

    void DrawTriangle(const Point& p1, const Point& p2, const Point& p3, const Pixel& pixel)
    {
        DrawLine(p1, p2, pixel);
        DrawLine(p2, p3, pixel);
        DrawLine(p3, p1, pixel);
    }

    void FillTriangle(const Point& p1, const Point& p2, const Point& p3, const Pixel& pixel)
    {
        auto* pp1 = &p1;
        auto* pp2 = &p2;
        auto* pp3 = &p3;

        if (pp1->y > pp2->y) std::swap(pp1, pp2);
        if (pp2->y > pp3->y) std::swap(pp2, pp3);
        if (pp1->y > pp2->y) std::swap(pp1, pp2);

        if (pp2->y == pp3->y)
        {
            FillBottomFlatTriangle(*pp1, *pp2, *pp3, pixel);
        }
        else if (pp1->y == pp2->y)
        {
            FillTopFlatTriangle(*pp1, *pp2, *pp3, pixel);
        }
        else
        {
            auto tmp = Point{
                static_cast<int>(pp1->x + (static_cast<float>(pp2->y - pp1->y) / static_cast<float>(pp3->y - pp1->y)) * (pp3->x - pp1->x)),
                pp2->y
            };
            FillBottomFlatTriangle(*pp1, *pp2, tmp, pixel);
            FillTopFlatTriangle(*pp2, tmp, *pp3, pixel);
        }
    }

private:
    void FillBottomFlatTriangle(const Point& p1, const Point& p2, const Point& p3, const Pixel& pixel)
    {
        float slope1 = static_cast<float>(p2.x - p1.x) / static_cast<float>(p2.y - p1.y);
        float slope2 = static_cast<float>(p3.x - p1.x) / static_cast<float>(p3.y - p1.y);

        float x1 = static_cast<float>(p1.x);
        float x2 = static_cast<float>(p1.x);

        for (int scanlineY = p1.y; scanlineY <= p2.y; scanlineY++)
        {
            DrawLine({ static_cast<int>(x1), scanlineY }, { static_cast<int>(x2), scanlineY }, pixel);
            x1 += slope1;
            x2 += slope2;
        }
    }

    void FillTopFlatTriangle(const Point& p1, const Point& p2, const Point& p3, const Pixel& pixel)
    {
        float slope1 = static_cast<float>(p3.x - p1.x) / static_cast<float>(p3.y - p1.y);
        float slope2 = static_cast<float>(p3.x - p2.x) / static_cast<float>(p3.y - p2.y);

        float x1 = static_cast<float>(p3.x);
        float x2 = static_cast<float>(p3.x);

        for (int scanlineY = p3.y; scanlineY > p1.y; scanlineY--)
        {
            DrawLine({ static_cast<int>(x1), scanlineY }, { static_cast<int>(x2), scanlineY }, pixel);
            x1 -= slope1;
            x2 -= slope2;
        }
    }

    void DrawLineLow(const Point& p1, const Point& p2, const Pixel& pixel)
    {
        int dx = p2.x - p1.x;
        int dy = p2.y - p1.y;

        int yi = 1;

        if (dy < 0)
        {
            yi = -1;
            dy = -dy;
        }

        int D = (2 * dy) - dx;
        int y = p1.y;

        for (int x = p1.x; x <= p2.x; ++x)
        {
            DrawPixel({ x, y }, pixel);
            if (D > 0)
            {
                y += yi;
                D += 2 * (dy - dx);
            }
            else
            {
                D += 2 * dy;
            }
        }
    }

    void DrawLineHigh(const Point& p1, const Point& p2, const Pixel& pixel)
    {
        int dx = p2.x - p1.x;
        int dy = p2.y - p1.y;

        int xi = 1;

        if (dx < 0)
        {
            xi = -1;
            dx = -dx;
        }

        int D = (2 * dx) - dy;
        int x = p1.x;

        for (int y = p1.y; y <= p2.y; ++y)
        {
            DrawPixel({ x, y }, pixel);
            if (D > 0)
            {
                x += xi;
                D += 2 * (dx - dy);
            }
            else
            {
                D += 2 * dx;
            }
        }
    }
};
