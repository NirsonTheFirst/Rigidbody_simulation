#pragma once

#include <cstddef>

inline constexpr double PhysicsHz = 60.0;
inline constexpr double FastHz = 240.0;
inline constexpr double LaunchSpeed = 2000.0;
inline constexpr double Restitution = 0.8;
inline constexpr double NormalRadius = 12.0;
inline constexpr double ShotRadius = 4.0;
inline constexpr double WallThickness = 10.0;
inline constexpr double Gravity = 460.0;

inline constexpr std::size_t StaticCount = 3;
inline constexpr std::size_t NormalCount = 16;
inline constexpr std::size_t ProjectileCount = 4;

static_assert(StaticCount >= 3 && StaticCount <= 10, "StaticCount must be between 3 and 10");
static_assert(NormalCount >= 10 && NormalCount <= 100, "NormalCount must be between 10 and 100");
static_assert(ProjectileCount >= 1 && ProjectileCount <= 10, "ProjectileCount must be between 1 and 10");
