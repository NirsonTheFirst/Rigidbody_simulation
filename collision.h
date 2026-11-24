#pragma once

struct Vec2
{
    double x = 0;
    double y = 0;

    Vec2 operator+(Vec2 b) const { return { x + b.x, y + b.y }; }
    Vec2 operator-(Vec2 b) const { return { x - b.x, y - b.y }; }
    Vec2 operator*(double k) const { return { x * k, y * k }; }
};

inline double dot(Vec2 a, Vec2 b)
{
    return a.x * b.x + a.y * b.y;
}

struct Circle
{
    Vec2 center;
    Vec2 velocity;
    double radius = 0;
    double invMass = 0;
};

struct Box
{
    Vec2 min;
    Vec2 max;
};

struct Hit
{
    double time;
    Vec2 normal;
};

bool circleTime(Vec2 delta, Vec2 relativeVelocity, double radius, Hit& hit);
bool wallTime(const Circle& circle, const Box& box, Hit& hit);
bool wallOverlap(const Circle& circle, const Box& box, Vec2& normal, double& depth);
