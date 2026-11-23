#pragma once

#include <array>

//расстояния в пикселях, время в секундах.
inline constexpr double PhysicsHz = 60.0;
inline constexpr double FastHz = 240.0;
inline constexpr double StartX = -12.0;
inline constexpr double LaunchSpeed = 2000.0;
inline constexpr double Restitution = 0.8;
inline constexpr double HalfWidth = 3.0;
inline constexpr double WallLeft = 0.0;
inline constexpr double WallRight = 10.0;

enum class Mode { Discrete, Ccd, MultiRate };

struct Shot
{
    double x = StartX;
    double velocity = LaunchSpeed;
    bool hit = false;
    double contactTime = 0.0;
    int substeps = 0;
};

Shot runExperiment(Mode mode);
bool checkExperiment(const std::array<Shot, 3>& results);
