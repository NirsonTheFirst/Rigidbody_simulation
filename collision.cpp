#include "collision.h"

#include <algorithm>
#include <array>
#include <cmath>

bool circleTime(Vec2 delta, Vec2 relativeVelocity, double radius, Hit& hit)
{
    const double a = dot(relativeVelocity, relativeVelocity);
    const double b = dot(delta, relativeVelocity);
    const double c = dot(delta, delta) - radius * radius;
    const double discriminant = b * b - a * c;
    if (a == 0 || b >= 0 || discriminant <= 0 || c < -1e-6)
        return false;

    const double time = std::max(0.0, c / (-b + std::sqrt(discriminant)));
    if (time > hit.time)
        return false;

    hit = { time, (delta + relativeVelocity * time) * (1.0 / radius) };
    return true;
}

bool wallTime(const Circle& circle, const Box& box, Hit& hit)
{
    bool found = false;
    const Vec2 p = circle.center;
    const Vec2 v = circle.velocity;
    const auto face = [&](double distance, double speed, double along, double alongSpeed,
        double low, double high, Vec2 normal)
    {
        if (speed == 0 || dot(v, normal) >= 0)
            return;
        const double time = distance / speed;
        const double point = along + alongSpeed * time;
        if (time >= -1e-9 && time <= hit.time && point >= low && point <= high)
        {
            hit = { std::max(0.0, time), normal };
            found = true;
        }
    };

    face(box.min.x - circle.radius - p.x, v.x, p.y, v.y, box.min.y, box.max.y, { -1, 0 });
    face(box.max.x + circle.radius - p.x, v.x, p.y, v.y, box.min.y, box.max.y, { 1, 0 });
    face(box.min.y - circle.radius - p.y, v.y, p.x, v.x, box.min.x, box.max.x, { 0, -1 });
    face(box.max.y + circle.radius - p.y, v.y, p.x, v.x, box.min.x, box.max.x, { 0, 1 });

    for (const double x : { box.min.x, box.max.x })
        for (const double y : { box.min.y, box.max.y })
        {
            Hit corner{ hit.time, {} };
            if (!circleTime(p - Vec2{ x, y }, v, circle.radius, corner))
                continue;
            const double sideX = x == box.min.x ? -1.0 : 1.0;
            const double sideY = y == box.min.y ? -1.0 : 1.0;
            if (corner.normal.x * sideX >= 0 && corner.normal.y * sideY >= 0)
            {
                hit = corner;
                found = true;
            }
        }
    return found;
}

bool wallOverlap(const Circle& circle, const Box& box, Vec2& normal, double& depth)
{
    const Vec2 closest{
        std::clamp(circle.center.x, box.min.x, box.max.x),
        std::clamp(circle.center.y, box.min.y, box.max.y)
    };
    const Vec2 offset = circle.center - closest;
    const double distance = std::sqrt(dot(offset, offset));
    if (distance >= circle.radius)
        return false;
    if (distance > 0)
    {
        normal = offset * (1.0 / distance);
        depth = circle.radius - distance;
        return true;
    }

    const std::array distances{
        circle.center.x - box.min.x, box.max.x - circle.center.x,
        circle.center.y - box.min.y, box.max.y - circle.center.y
    };
    const std::array<Vec2, 4> normals{ Vec2{ -1, 0 }, { 1, 0 }, { 0, -1 }, { 0, 1 } };
    const auto nearest = std::min_element(distances.begin(), distances.end());
    normal = normals[nearest - distances.begin()];
    depth = circle.radius + *nearest;
    return true;
}
