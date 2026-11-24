#pragma once

#include "simulation.h"
#include <SFML/Graphics.hpp>

struct FrameStats
{
    double totalPhysicsMs = 0;
    int frames = 0;
    int fastSteps = 0;
};

void drawWorld(sf::RenderTarget& target, const sf::Font& font, bool hasFont, const World& world,
    const World& previous, Mode mode, bool running, int speedDivisor, double alpha,
    const FrameStats& stats);
