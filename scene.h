#pragma once

#include "collision.h"

#include <array>
#include <cstddef>
#include <vector>

inline constexpr Box Arena{ { 28, 100 }, { 992, 442 } };

struct World
{
    std::vector<Circle> normal;
    std::vector<Circle> fast;
    std::vector<Circle> fixed;
    std::array<Box, 5> walls;
    bool gravityEnabled = true;
};

inline constexpr std::size_t TestWall = 0;
inline constexpr std::size_t Floor = 1;

World makeWorld();
