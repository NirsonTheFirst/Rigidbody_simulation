#include "display.h"

#include <string>

static float screenX(double x)
{
    return 616.0f + static_cast<float>(x * 11.0);
}

static void rectangle(sf::RenderWindow& window, sf::Vector2f position, sf::Vector2f size,
    sf::Color fill, sf::Color outline = sf::Color::Transparent)
{
    sf::RectangleShape shape(size);
    shape.setPosition(position);
    shape.setFillColor(fill);
    shape.setOutlineThickness(outline == sf::Color::Transparent ? 0.0f : 1.0f);
    shape.setOutlineColor(outline);
    window.draw(shape);
}

static void label(sf::RenderWindow& window, const sf::Font& font, const std::wstring& value,
    sf::Vector2f position, unsigned int size, sf::Color color)
{
    sf::Text text(font, value, size);
    text.setPosition(position);
    text.setFillColor(color);
    window.draw(text);
}

constexpr std::array ModeLabels{ L"Обычная проверка", L"CCD", L"Multi-rate" };

static void drawDirection(sf::RenderWindow& window, float x, float y, bool right, sf::Color color)
{
    const float sign = right ? 1.0f : -1.0f;
    const float tip = x + sign * 27.0f;
    const std::array<sf::Vertex, 5> points{{
        { { x + sign * 7.0f, y }, color },
        { { tip, y }, color },
        { { tip - sign * 6.0f, y - 5.0f }, color },
        { { tip, y }, color },
        { { tip - sign * 6.0f, y + 5.0f }, color }
    }};
    window.draw(points.data(), points.size(), sf::PrimitiveType::LineStrip);
}

void drawExperiment(sf::RenderWindow& window, const sf::Font& font, bool hasFont,
    const std::array<Shot, 3>& results, bool showResult, int visibleMode)
{
    const sf::Color Paper = sf::Color::White;
    const sf::Color Ink = sf::Color::Black;
    constexpr sf::Color Muted{ 100, 100, 100 };
    constexpr sf::Color Rule{ 210, 210, 210 };
    constexpr sf::Color Wall{ 175, 175, 175 };
    constexpr sf::Color HitColor{ 34, 115, 62 };
    constexpr sf::Color MissColor{ 170, 45, 45 };

    window.clear(Paper);
    if (hasFont)
    {
        label(window, font, L"Столкновение со стенкой", { 28, 20 }, 25, Ink);
        label(window, font, L"Скорость: " + std::to_wstring(static_cast<int>(LaunchSpeed)) +
            L" пикс/с, стенка " + std::to_wstring(static_cast<int>(WallRight - WallLeft)) +
            L" пикс.", { 29, 55 }, 17, Muted);
        label(window, font, showResult ? L"После шага" : L"До шага",
            { 770, 27 }, 19, Ink);
        label(window, font, L"Старт", { screenX(StartX) - 22, 93 }, 16, Muted);
        label(window, font, L"Стенка", { screenX((WallLeft + WallRight) / 2) - 24, 93 }, 16, Muted);
        label(window, font, showResult ? L"Контур: начальное положение. Стрелка: направление движения."
            : L"Нажмите пробел, чтобы сделать шаг.",
            { 29, 457 }, 17, Ink);
        label(window, font, L"Пробел: шаг     R: сброс     0: все режимы     1-3: один режим",
            { 29, 491 }, 16, Muted);
    }
    for (std::size_t i = 0; i < results.size(); ++i)
    {
        if (visibleMode >= 0 && visibleMode != static_cast<int>(i))
            continue;
        const float y = visibleMode < 0 ? 165.0f + 109.0f * static_cast<float>(i) : 274.0f;
        const Shot shot = showResult ? results[i] : Shot{};
        rectangle(window, { 28, y - 50 }, { 964, 1 }, Rule);
        rectangle(window, { 280, y }, { 685, 1 }, Rule);
        rectangle(window, { screenX(WallLeft), y - 33 },
            { screenX(WallRight) - screenX(WallLeft), 66 }, Wall);

        const auto position = [y](double x) { return sf::Vector2f{ screenX(x - HalfWidth), y - 12 }; };
        const sf::Vector2f shotSize{ screenX(HalfWidth) - screenX(-HalfWidth), 24 };
        if (showResult)
            rectangle(window, position(StartX), shotSize, sf::Color::Transparent, Muted);

        const bool tunnelled = showResult && !shot.hit && shot.x - HalfWidth >= WallRight;
        const sf::Color resultColor = shot.hit ? HitColor : (tunnelled ? MissColor : Ink);
        rectangle(window, position(shot.x), shotSize, resultColor);
        const bool movingRight = shot.velocity >= 0;
        const float arrowX = screenX(shot.x + (movingRight ? HalfWidth : -HalfWidth));
        drawDirection(window, arrowX, y, movingRight, resultColor);

        if (hasFont)
        {
            label(window, font, ModeLabels[i], { 29, y - 30 }, 20, Ink);
            const std::wstring frequency = i == 2
                ? std::to_wstring(static_cast<int>(FastHz)) + L" Гц для быстрого тела"
                : std::to_wstring(static_cast<int>(PhysicsHz)) + L" Гц";
            label(window, font, frequency, { 29, y - 4 }, 16, Muted);
            if (showResult)
            {
                label(window, font, shot.hit ? L"Отскочил" :
                    (tunnelled ? L"Пролетел сквозь стенку" : L"Продолжил движение"),
                    { 29, y + 17 }, 17, resultColor);
            }
        }
    }
}
