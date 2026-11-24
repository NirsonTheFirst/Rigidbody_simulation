#include "display.h"
#include "settings.h"

#include <iomanip>
#include <sstream>
#include <string>

namespace
{
const sf::Color Ink{ 32, 32, 32 };
const sf::Color Grey{ 106, 106, 106 };
const std::wstring NormalHz = std::to_wstring(static_cast<int>(PhysicsHz));
const std::wstring ProjectileHz = std::to_wstring(static_cast<int>(FastHz));
struct ModeLabel
{
    Mode mode;
    std::wstring text;
};

const std::array ModeLabels{
    ModeLabel{ Mode::Discrete, L"1   Discrete (" + NormalHz + L" Hz)" },
    ModeLabel{ Mode::Ccd, L"2   Continuous (CCD)" },
    ModeLabel{ Mode::MultiRate, L"3   Multi-rate (" + NormalHz + L"/" + ProjectileHz + L" Hz)" }
};

void label(sf::RenderTarget& target, const sf::Font& font, const std::wstring& value,
    sf::Vector2f position, unsigned size, sf::Color color = Ink)
{
    sf::Text text(font, value, size);
    text.setPosition(position);
    text.setFillColor(color);
    target.draw(text);
}

void rectangle(sf::RenderTarget& target, sf::Vector2f position, sf::Vector2f size, sf::Color color)
{
    sf::RectangleShape rect(size);
    rect.setPosition(position);
    rect.setFillColor(color);
    target.draw(rect);
}

void drawPack(sf::RenderTarget& target, const std::vector<Circle>& pack, const std::vector<Circle>& previous,
    double alpha, sf::Color color)
{
    for (std::size_t i = 0; i < pack.size(); ++i)
    {
        const Circle& body = pack[i];
        const Vec2 center = previous[i].center + (body.center - previous[i].center) * alpha;
        const float radius = static_cast<float>(body.radius);
        sf::CircleShape shape(radius);
        shape.setOrigin({ radius, radius });
        shape.setPosition({ static_cast<float>(center.x), static_cast<float>(center.y) });
        shape.setFillColor(color);
        target.draw(shape);
    }
}
}

void drawWorld(sf::RenderTarget& target, const sf::Font& font, bool hasFont, const World& world,
    const World& previous, Mode mode, bool running, int speedDivisor, double alpha,
    const FrameStats& stats)
{
    target.clear(sf::Color::White);
    constexpr float rim = 12;
    const sf::Vector2f origin{ static_cast<float>(Arena.min.x), static_cast<float>(Arena.min.y) };
    const sf::Vector2f size{ static_cast<float>(Arena.max.x - Arena.min.x),
        static_cast<float>(Arena.max.y - Arena.min.y) };
    rectangle(target, origin - sf::Vector2f{ rim, rim }, size + sf::Vector2f{ 2 * rim, 2 * rim }, Grey);
    rectangle(target, origin, size, sf::Color(247, 247, 247));

    const Box& wall = world.walls[TestWall];
    rectangle(target, { static_cast<float>(wall.min.x), static_cast<float>(wall.min.y) },
        { static_cast<float>(wall.max.x - wall.min.x), static_cast<float>(wall.max.y - wall.min.y) },
        sf::Color(138, 138, 138));

    drawPack(target, world.fixed, previous.fixed, alpha, sf::Color(193, 193, 193));
    drawPack(target, world.normal, previous.normal, alpha, Grey);
    drawPack(target, world.fast, previous.fast, alpha, Ink);

    if (!hasFont)
        return;

    label(target, font, L"Rigidbody Simulation", { 27, 14 }, 25);
    label(target, font, world.gravityEnabled ? L"Gravity: on" : L"Gravity: off",
        { 669, 20 }, 16, Grey);
    const std::wstring speed = speedDivisor == 1 ? L"1" : L"1/" + std::to_wstring(speedDivisor);
    label(target, font, L"Speed: " + speed, { 480, 20 }, 16, Grey);
    label(target, font, running ? L"Running" : L"Paused", { 851, 20 }, 16, Grey);
    for (std::size_t i = 0; i < ModeLabels.size(); ++i)
    {
        const float x = 28.f + 213.f * static_cast<float>(i);
        const bool selected = ModeLabels[i].mode == mode;
        rectangle(target, { x, 55 }, { 197, 31 }, selected ? Ink : sf::Color(233, 233, 233));
        label(target, font, ModeLabels[i].text, { x + 11, 61 }, 16, selected ? sf::Color::White : Ink);
    }

    label(target, font, std::to_wstring(world.fixed.size()) + L" static circles    " +
        std::to_wstring(world.normal.size()) + L" regular circles    " + std::to_wstring(world.fast.size()) +
        L" projectiles    Wall: " + std::to_wstring(static_cast<int>(WallThickness)) + L" px",
        { 28, 471 }, 15, Grey);
    std::wostringstream physicsTime;
    physicsTime << L"Physics avg: ";
    if (stats.frames > 0)
        physicsTime << std::fixed << std::setprecision(3) << stats.totalPhysicsMs / stats.frames;
    else
        physicsTime << L"--";
    physicsTime << L" ms/frame";
    label(target, font, physicsTime.str(), { 572, 471 }, 15);
    if (mode == Mode::MultiRate)
        label(target, font, L"240 Hz steps: " + (stats.frames > 0 ? std::to_wstring(stats.fastSteps) : L"--"),
            { 823, 471 }, 15);

    label(target, font, L"Space: step    P: play/pause    S: slow motion    G: gravity    R: reset    Esc: exit",
        { 28, 503 }, 15);
}
