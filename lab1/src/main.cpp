#include <vector>
#include <queue>
#include <array>
#include <utility>

#include "SFML/Graphics.hpp"

using namespace std;
using namespace sf;

struct Pixel
{
    uint8_t r, g, b, a;

    friend bool operator==(const Pixel& lhs, const Pixel& rhs)
    {
        return lhs.r == rhs.r && lhs.g == rhs.g && lhs.b == rhs.b && lhs.a == rhs.a;
    }

    friend bool operator!=(const Pixel& lhs, const Pixel& rhs)
    {
        return !(lhs == rhs);
    }
};

struct Bitmap
{
    vector<Pixel> pixels;
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

    bool Contains(Vector2i position) const
    {
        int x = position.x;
        int y = position.y;
        return x >= 0 && y >= 0 && x < width && y < height;
    }

    bool SetPixel(Vector2i position, Pixel pixel)
    {
        int x = position.x;
        int y = position.y;
        if (!Contains(position))
        {
            return false;
        }
        pixels[y * width + x] = pixel;
        return true;
    }

    Pixel GetPixelAt(Vector2i position) const
    {
        if (!Contains(position))
        {
            return Pixel{ 0, 0, 0, 0 };
        }
        return pixels[position.y * width + position.x];
    }

    void Clear()
    {
        for (auto& p : pixels)
        {
            p = { 0, 0, 0, 255 };
        }
    }

    bool FillShape(Vector2i start, Pixel fillPixel)
    {
        if (!Contains(start))
        {
            return false;
        }

        Pixel initialPixel = GetPixelAt(start);

        if (initialPixel == fillPixel)
        {
            return true;
        }

        SetPixel(start, fillPixel);

        queue<Vector2i> queue;
        queue.push(start);

        constexpr array<pair<int, int>, 4> searchDirections = {{ { 0, -1 }, { 1, 0 }, { 0, 1 }, { -1, 0 } }};

        while (!queue.empty())
        {
            Vector2i currentPosition = queue.front();
            queue.pop();

            for (auto& [dx, dy] : searchDirections)
            {
                Vector2i nextPosition = { currentPosition.x + dx, currentPosition.y + dy };
                if (!Contains(nextPosition))
                {
                    continue;
                }
                Pixel currentPixel = GetPixelAt(nextPosition);
                if (currentPixel != initialPixel)
                {
                    continue;
                }
                SetPixel(nextPosition, fillPixel);
                queue.push(nextPosition);
            }
        }

        return true;
    }
};

void UpdateTextureFromBitmap(Texture& texture, const Bitmap& bitmap)
{
    texture.update(reinterpret_cast<const Uint8*>(bitmap.pixels.data()));
}

Vector2i GetBitmapCursorPostion(const Window& window, float screenPixelToBitmapPixelRatio)
{
    Vector2i cursorPosition = Mouse::getPosition(window);
    return Vector2i(
        static_cast<int>(cursorPosition.x / screenPixelToBitmapPixelRatio),
        static_cast<int>(cursorPosition.y / screenPixelToBitmapPixelRatio)
    );
}

Pixel ColorToPixel(const Color& color)
{
    return { color.r, color.g, color.b, 255 };
}

int main()
{
    vector<Color> pallete = { Color::Red, Color::Green, Color::Blue, Color::Yellow, Color::Cyan };
    vector<RectangleShape> colorMenu;
    float wigetBorder = 30.0f;
    float wigetOffset = 10.0f;
    float wigetGap = 7.0f;
    for (int i = 0; i < pallete.size(); ++i)
    {
        Color& color = pallete[i];
        RectangleShape wiget;
        wiget.setFillColor(color);
        wiget.setSize({ wigetBorder, wigetBorder });
        wiget.setPosition({ wigetOffset + i * (wigetBorder + wigetGap), wigetOffset });
        colorMenu.push_back(wiget);
    }

    float selectionWigetOverflow = 5.0f;
    RectangleShape selectionWiget;
    selectionWiget.setSize({ wigetBorder + selectionWigetOverflow, wigetBorder + selectionWigetOverflow });
    selectionWiget.setFillColor(Color::White);

    Pixel selectedPixel;
    int selectedColor = 0;

    auto adjustSelectionWiget = [&]()
    {
        auto pos = colorMenu[selectedColor].getPosition();
        selectionWiget.setPosition({ pos.x - selectionWigetOverflow / 2.0f, pos.y - selectionWigetOverflow / 2.0f });
        selectedPixel = ColorToPixel(pallete[selectedColor]);
    };

    adjustSelectionWiget();

    RenderWindow window(VideoMode(800, 600), "Area fill alogrithm");

    Vector2f windowSize = window.getView().getSize();
    float screenPixelToBitmapPixelRatio = 8;
    Bitmap bitmap = Bitmap::New(
        static_cast<int>(windowSize.x / screenPixelToBitmapPixelRatio),
        static_cast<int>(windowSize.y / screenPixelToBitmapPixelRatio)
    );

    Texture texture;
    texture.create(bitmap.width, bitmap.height);

    RectangleShape screen;
    screen.setSize(windowSize);
    screen.setTexture(&texture, false);

    bool shiftWasPressed = false;

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
       
        if (Mouse::isButtonPressed(Mouse::Button::Left))
        {
            bitmap.SetPixel(GetBitmapCursorPostion(window, screenPixelToBitmapPixelRatio), selectedPixel);
        }
        if (Mouse::isButtonPressed(Mouse::Button::Right))
        {
            bitmap.FillShape(GetBitmapCursorPostion(window, screenPixelToBitmapPixelRatio), selectedPixel);
        }
        if (Keyboard::isKeyPressed(Keyboard::Space))
        {
            bitmap.Clear();
        }

        bool shiftIsPressed = Keyboard::isKeyPressed(Keyboard::LShift);
        if (shiftIsPressed && !shiftWasPressed)
        {
            shiftWasPressed = true;
            selectedColor = (selectedColor + 1) % pallete.size();
            adjustSelectionWiget();
        }
        else if (!shiftIsPressed && shiftWasPressed)
        {
            shiftWasPressed = false;
        }

        window.clear();
        UpdateTextureFromBitmap(texture, bitmap);
        window.draw(screen);
        window.draw(selectionWiget);
        for (auto& wiget : colorMenu)
        {
            window.draw(wiget);
        }
        window.display();
    }

    return 0;
}
