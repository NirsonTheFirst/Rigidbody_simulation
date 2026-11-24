#pragma once

#include "scene.h"

enum class Mode { Discrete, Ccd, MultiRate };

int stepWorld(World& world, Mode mode);
