#include "simulation.h"
#include "settings.h"

#include <algorithm>
#include <cmath>

constexpr int SolverPasses = 24;
constexpr int MaxCcdContacts = 64;
constexpr double FloorFriction = 0.16;

static void impulse(Circle& a, Circle* b, Vec2 normal)
{
    const double inward = dot(a.velocity - (b ? b->velocity : Vec2{}), normal);
    if (inward >= 0)
        return;

    const double invB = b ? b->invMass : 0;
    const double bounce = inward < -35 ? Restitution : 0; //мелкие удары не раскачивают стопку
    const double j = -(1 + bounce) * inward / (a.invMass + invB);
    a.velocity = a.velocity + normal * (j * a.invMass);
    if (b)
        b->velocity = b->velocity - normal * (j * invB);
    else if (normal.y < -0.9)
    {
        //трение от опоры уменьшает скорость вдоль пола до нуля
        const double slowdown = FloorFriction * j * a.invMass;
        a.velocity.x -= std::clamp(a.velocity.x, -slowdown, slowdown);
    }
}

static bool separate(Circle& a, Circle& b, bool holdB = false)
{
    const Vec2 delta = a.center - b.center;
    const double radius = a.radius + b.radius;
    const double lengthSq = dot(delta, delta);
    if (lengthSq >= radius * radius - 1e-7)
        return false;
    const double length = std::sqrt(lengthSq);
    const Vec2 normal = length > 0 ? delta * (1.0 / length) : Vec2{ -1, 0 };

    const double positionInvMassB = holdB ? 0 : b.invMass;
    const double correction = (radius - length) / (a.invMass + positionInvMassB);
    a.center = a.center + normal * (correction * a.invMass);
    if (!holdB)
        b.center = b.center - normal * (correction * b.invMass);
    impulse(a, &b, normal);
    return true;
}

static bool solveTestWall(World& world, Circle& circle)
{
    Vec2 normal;
    double depth = 0;
    if (!wallOverlap(circle, world.walls[TestWall], normal, depth) || depth < 1e-8)
        return false;
    circle.center = circle.center + normal * depth;
    impulse(circle, nullptr, normal);
    return true;
}

static bool keepInside(Circle& circle)
{
    const double r = circle.radius;
    const Vec2 inside{
        std::clamp(circle.center.x, Arena.min.x + r, Arena.max.x - r),
        std::clamp(circle.center.y, Arena.min.y + r, Arena.max.y - r)
    };

    const Vec2 correction = inside - circle.center;
    if (correction.x != 0)
        impulse(circle, nullptr, { std::copysign(1.0, correction.x), 0 });
    if (correction.y != 0)
        impulse(circle, nullptr, { 0, std::copysign(1.0, correction.y) });
    circle.center = inside;
    return correction.x != 0 || correction.y != 0;
}

static bool keepPackInside(std::vector<Circle>& pack)
{
    bool corrected = false;
    for (Circle& circle : pack)
        corrected |= keepInside(circle);
    return corrected;
}

static bool solvePack(World& world, std::vector<Circle>& pack)
{
    bool overlap = false;
    for (std::size_t i = 0; i < pack.size(); ++i)
    {
        Circle& a = pack[i];
        overlap |= solveTestWall(world, a);
        for (Circle& fixed : world.fixed)
            overlap |= separate(a, fixed);
        for (std::size_t j = i + 1; j < pack.size(); ++j)
            overlap |= separate(a, pack[j]);
    }
    return overlap;
}

static bool solveMixed(World& world, bool holdNormal = false)
{
    bool overlap = false;
    for (Circle& a : world.fast)
        for (Circle& b : world.normal)
            overlap |= separate(a, b, holdNormal);
    return overlap;
}

static void advance(std::vector<Circle>& pack, double dt)
{
    for (Circle& c : pack)
        c.center = c.center + c.velocity * dt;
}

static void advanceShots(World& world, double dt)
{
    const Box& wall = world.walls[TestWall];
    for (Circle& shot : world.fast)
    {
        const double previousX = shot.center.x;
        shot.center = shot.center + shot.velocity * dt;

        const double left = wall.min.x - shot.radius;
        const double right = wall.max.x + shot.radius;
        if (shot.center.x <= left || shot.center.x >= right ||
            shot.center.y < wall.min.y || shot.center.y > wall.max.y)
            continue;

        if (previousX <= left)
        {
            shot.center.x = left;
            impulse(shot, nullptr, { -1, 0 });
        }
        else if (previousX >= right)
        {
            shot.center.x = right;
            impulse(shot, nullptr, { 1, 0 });
        }
    }
}

static void settleAll(World& world)
{
    for (int pass = 0; pass < SolverPasses; ++pass)
    {
        bool overlap = solvePack(world, world.normal);
        overlap |= solvePack(world, world.fast);
        overlap |= solveMixed(world);
        overlap |= keepPackInside(world.normal);
        overlap |= keepPackInside(world.fast);
        if (!overlap)
            break;
    }
}

struct Contact
{
    Circle* a = nullptr;
    Circle* b = nullptr; //nullptr означает плоскую стенку
    Hit hit;
};

static void stepCcd(World& world, double dt)
{
    settleAll(world); //сначала убираем перекрытия, оставшиеся к началу шага
    double remaining = dt;
    for (int contact = 0; contact < MaxCcdContacts && remaining > 1e-9; ++contact)
    {
        Contact first{ nullptr, nullptr, { remaining, {} } };
        const auto findPair = [&](Circle& a, Circle& b)
        {
            const Vec2 relative = a.velocity - b.velocity;
            Hit hit{ first.hit.time, {} };
            if (circleTime(a.center - b.center, relative, a.radius + b.radius, hit) &&
                dot(relative, hit.normal) < -1)
                first = { &a, &b, hit };
        };
        const auto findWalls = [&](Circle& c)
        {
            for (const Box& wall : world.walls)
            {
                Hit hit{ first.hit.time, {} };
                if (wallTime(c, wall, hit) && dot(c.velocity, hit.normal) < -1)
                    first = { &c, nullptr, hit };
            }
        };

        for (std::size_t i = 0; i < world.normal.size(); ++i)
        {
            Circle& a = world.normal[i];
            findWalls(a);
            for (std::size_t j = i + 1; j < world.normal.size(); ++j)
                findPair(a, world.normal[j]);
            for (Circle& fixed : world.fixed)
                findPair(a, fixed);
        }
        for (std::size_t i = 0; i < world.fast.size(); ++i)
        {
            Circle& a = world.fast[i];
            findWalls(a);
            for (std::size_t j = i + 1; j < world.fast.size(); ++j)
                findPair(a, world.fast[j]);
            for (Circle& b : world.normal)
                findPair(a, b);
            for (Circle& fixed : world.fixed)
                findPair(a, fixed);
        }

        //все тела доходят до одного момента контакта, затем считаем остаток шага.
        advance(world.normal, first.hit.time);
        advance(world.fast, first.hit.time);
        remaining -= first.hit.time;
        if (!first.a)
            break;
        impulse(*first.a, first.b, first.hit.normal);
    }
    advance(world.normal, remaining);
    advance(world.fast, remaining);
    settleAll(world);
}

static void applyGravity(std::vector<Circle>& pack, double dt, bool enabled)
{
    if (enabled)
        for (Circle& c : pack)
            c.velocity.y += Gravity * dt;
}

static void applyGravity(World& world, double dt)
{
    applyGravity(world.normal, dt, world.gravityEnabled);
    applyGravity(world.fast, dt, world.gravityEnabled);
}

static void stepDiscrete(World& world, double dt)
{
    applyGravity(world, dt);
    advance(world.normal, dt);
    advanceShots(world, dt);
    settleAll(world);
}

static int stepMultiRate(World& world, double dt)
{
    applyGravity(world.normal, dt, world.gravityEnabled);
    advance(world.normal, dt);
    for (int pass = 0; pass < SolverPasses; ++pass)
    {
        bool overlap = solvePack(world, world.normal);
        overlap |= keepPackInside(world.normal);
        if (!overlap)
            break;
    }

    int substeps = 0;
    for (double timeLeft = dt; timeLeft > 1e-9; ++substeps)
    {
        const double step = std::min(timeLeft, 1.0 / FastHz);
        applyGravity(world.fast, step, world.gravityEnabled);
        advanceShots(world, step);
        for (int pass = 0; pass < SolverPasses; ++pass)
        {
            bool overlap = solvePack(world, world.fast);
            //импульс получают оба тела
            overlap |= solveMixed(world, true);
            overlap |= keepPackInside(world.fast);
            if (!overlap)
                break;
        }
        timeLeft -= step;
    }
    return substeps;
}

int stepWorld(World& world, Mode mode)
{
    constexpr double dt = 1.0 / PhysicsHz;
    switch (mode)
    {
    case Mode::Discrete:
        stepDiscrete(world, dt);
        break;
    case Mode::Ccd:
        applyGravity(world, dt);
        stepCcd(world, dt);
        break;
    case Mode::MultiRate:
        return stepMultiRate(world, dt);
    }
    return 0;
}
