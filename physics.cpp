#include "physics.h"

#include <algorithm>
#include <cmath>
#include <iostream>

//проверяем пересечение со стенкой в конце шага.
static void discreteStep(Shot& shot, double dt)
{
    const double oldX = shot.x;
    shot.x += shot.velocity * dt;
    const bool overlaps = shot.x + HalfWidth > WallLeft && shot.x - HalfWidth < WallRight;
    const bool enteredFromLeft = oldX + HalfWidth <= WallLeft && shot.velocity > 0.0;
    if (overlaps && enteredFromLeft)
    {
        shot.x = WallLeft - HalfWidth;
        shot.velocity *= -Restitution;
        shot.hit = true;
    }
}

static void ccdStep(Shot& shot, double dt)
{
    if (shot.velocity > 0.0)
    {
        const double time = (WallLeft - (shot.x + HalfWidth)) / shot.velocity; //зазор до стенки / скорость
        if (time >= 0.0 && time <= dt)
        {
            shot.x = WallLeft - HalfWidth;
            shot.velocity *= -Restitution;
            shot.x += shot.velocity * (dt - time); //остаток шага после отскока
            shot.hit = true;
            shot.contactTime = time;
            return;
        }
    }
    shot.x += shot.velocity * dt;
}

Shot runExperiment(Mode mode)
{
    Shot shot;
    const double dt = 1.0 / PhysicsHz;
    if (mode == Mode::Ccd)
    {
        ccdStep(shot, dt);
    }
    else if (mode == Mode::MultiRate)
    {
        double remaining = dt;
        while (remaining > 1e-12)
        {
            const double substep = std::min(remaining, 1.0 / FastHz); //последний подшаг может быть короче
            discreteStep(shot, substep);
            remaining -= substep;
            ++shot.substeps;
        }
    }
    else
    {
        discreteStep(shot, dt);
    }
    return shot;
}

bool checkExperiment(const std::array<Shot, 3>& results)
{
    constexpr std::array titles{ "Discrete", "CCD", "Multi-rate" };
    for (std::size_t i = 0; i < results.size(); ++i)
    {
        const Shot& shot = results[i];
        std::cout << titles[i] << ": x=" << shot.x << " v=" << shot.velocity
            << " hit=" << shot.hit << " contact_ms=" << shot.contactTime * 1000.0
            << " substeps=" << shot.substeps << '\n';
    }

    const Shot& discrete = results[0];
    const Shot& ccd = results[1];
    const Shot& multiRate = results[2];
    return !discrete.hit && discrete.x - HalfWidth > WallRight
        && ccd.hit && ccd.velocity < 0 && std::abs(ccd.contactTime - 0.0045) < 1e-9
        && multiRate.hit && multiRate.velocity < 0 && multiRate.substeps == 4;
}
