#pragma once

#include "physics.h"

#include <SFML/Graphics.hpp>

void drawExperiment(sf::RenderWindow& window, const sf::Font& font, bool hasFont,
    const std::array<Shot, 3>& results, bool showResult, int visibleMode);
