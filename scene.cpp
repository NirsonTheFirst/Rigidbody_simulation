#include "scene.h"
#include "settings.h"

#include <cmath>
#include <stdexcept>

namespace
{
constexpr std::size_t StackSize = 4;
constexpr Vec2 TestShotStart{ 487, 150 };
constexpr double SpawnClearance = 4;

bool canSpawn(const World& world, Vec2 center, double radius)
{
    const Box& wall = world.walls[TestWall];
    if (center.x + radius + SpawnClearance >= wall.min.x && center.x - radius - SpawnClearance <= wall.max.x)
        return false;

    const double shotTravel = std::abs(LaunchSpeed) / PhysicsHz;
    if (std::abs(center.y - TestShotStart.y) < radius + ShotRadius + SpawnClearance &&
        std::abs(center.x - TestShotStart.x) < shotTravel + radius + ShotRadius + SpawnClearance)
        return false;

    for (const auto* pack : { &world.fixed, &world.normal, &world.fast })
        for (const Circle& circle : *pack)
        {
            const Vec2 delta = center - circle.center;
            const double distance = radius + circle.radius + SpawnClearance;
            if (dot(delta, delta) < distance * distance)
                return false;
        }
    return true;
}

void fillGroup(World& world, std::vector<Circle>& pack, std::size_t count,
    double radius, double speed, double invMass)
{
    const double margin = radius + SpawnClearance;
    const double spacing = 2 * margin;
    for (double y = Arena.min.y + margin; y <= Arena.max.y - margin && pack.size() < count; y += spacing)
        for (double x = Arena.min.x + margin; x <= Arena.max.x - margin && pack.size() < count; x += spacing)
            if (canSpawn(world, { x, y }, radius))
            {
                const double vx = x < (Arena.min.x + Arena.max.x) / 2 ? speed : -speed;
                pack.push_back({ { x, y }, { vx, 0 }, radius, invMass });
            }

    if (pack.size() != count)
        throw std::runtime_error("The requested circles do not fit in the arena");
}

}

World makeWorld()
{
    constexpr double borderThickness = 120;
    const auto [left, top] = Arena.min;
    const auto [right, bottom] = Arena.max;
    World world;
    world.walls = {{
        { { 505, top }, { 505 + WallThickness, bottom } },
        { { left, bottom }, { right, bottom + borderThickness } },
        { { left, top - borderThickness }, { right, top } },
        { { left - borderThickness, top }, { left, bottom } },
        { { right, top }, { right + borderThickness, bottom } }
    }};
    for (const Vec2 center : { Vec2{ 215, 180 }, { 535, 285 }, { 915, 318 } })
        world.fixed.push_back({ center, {}, 2 * NormalRadius, 0 });

    for (std::size_t i = 0; i < StackSize; ++i)
        world.normal.push_back({ { 112, world.walls[Floor].min.y - NormalRadius * (1 + 2 * i) },
            {}, NormalRadius, 1 });
    for (const Vec2 center : { Vec2{ 95, 161 }, { 160, 259 }, { 280, 199 }, { 318, 346 },
        { 399, 209 }, { 448, 365 }, { 594, 182 }, { 635, 304 }, { 716, 351 }, { 914, 219 } })
    {
        if (world.normal.size() == NormalCount - 2)
            break;
        world.normal.push_back({ center, { center.x < 500 ? 56.0 : -38.0, 0 }, NormalRadius, 1 });
    }
    world.normal.push_back({ { 738, 182 }, { 110, 0 }, NormalRadius, 1 });
    world.normal.push_back({ { 846, 182 }, { -110, 0 }, NormalRadius, 1 });

    world.fast = {
        { TestShotStart, { LaunchSpeed, 0 }, ShotRadius, 1 },
        { { 357, 263 }, { 1200, 0 }, ShotRadius, 1 },
        { { 402, 263 }, { -1200, 0 }, ShotRadius, 1 },
        { { 580, 304 }, { 1200, 0 }, ShotRadius, 1 }
    };
    if (world.fast.size() > ProjectileCount)
        world.fast.resize(ProjectileCount);

    fillGroup(world, world.fixed, StaticCount, 2 * NormalRadius, 0, 0);
    fillGroup(world, world.normal, NormalCount, NormalRadius, 56, 1);
    fillGroup(world, world.fast, ProjectileCount, ShotRadius, 1200, 1);
    return world;
}
