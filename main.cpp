#include "display.h"
#include "settings.h"
#include "simulation.h"

#include <algorithm>
#include <chrono>

int main()
{
    sf::RenderWindow window(sf::VideoMode({ 1020, 560 }), L"Rigidbody Simulation",
        sf::Style::Titlebar | sf::Style::Close);
    window.setFramerateLimit(60);
    window.setKeyRepeatEnabled(false);
    sf::Font font;
    const bool hasFont = font.openFromFile("C:/Windows/Fonts/arial.ttf");

    World world = makeWorld();
    World previous = world;
    Mode mode = Mode::Discrete;
    bool running = false;
    bool slowMotion = false;
    sf::Clock clock;
    double accumulator = 0;
    FrameStats stats;
    FrameStats lastMeasured;
    constexpr double dt = 1.0 / PhysicsHz;

    const auto clearMeasurements = [&]
    {
        stats = {};
        lastMeasured = {};
    };
    const auto reset = [&]
    {
        const bool gravityEnabled = world.gravityEnabled;
        world = makeWorld();
        world.gravityEnabled = gravityEnabled;
        previous = world;
        accumulator = 0;
        running = false;
        stats = {};
    };
    const auto select = [&](Mode selected)
    {
        if (mode != selected)
            lastMeasured = {};
        mode = selected;
        reset(); //все три режима начинаются с одинаковой сцены
    };

    while (window.isOpen())
    {
        const double elapsed = std::min(0.1, static_cast<double>(clock.restart().asSeconds()));
        bool singleStep = false;
        while (const auto event = window.pollEvent())
        {
            if (event->is<sf::Event::Closed>())
                window.close();
            const auto* key = event->getIf<sf::Event::KeyPressed>();
            if (!key)
                continue;
            using Key = sf::Keyboard::Key;
            if (key->code == Key::Escape) window.close();
            if (key->code == Key::Num1) select(Mode::Discrete);
            if (key->code == Key::Num2) select(Mode::Ccd);
            if (key->code == Key::Num3) select(Mode::MultiRate);
            if (key->code == Key::R) reset();
            if (key->code == Key::S)
            {
                slowMotion = !slowMotion;
                clearMeasurements();
            }
            if (key->code == Key::G)
            {
                world.gravityEnabled = !world.gravityEnabled;
                clearMeasurements();
            }
            if (key->code == Key::Space && !running) singleStep = true;
            if (key->code == Key::P)
            {
                running = !running;
                previous = world;
                accumulator = 0;
            }
        }
        if (!window.isOpen())
            break;

        const int speedDivisor = slowMotion ? 16 : 1;
        double physicsMs = 0;
        int fastSteps = 0;
        bool stepped = false;
        const auto simulateStep = [&]
        {
            previous = world;
            const auto start = std::chrono::steady_clock::now();
            const int steps = stepWorld(world, mode);
            const auto end = std::chrono::steady_clock::now();
            physicsMs += std::chrono::duration<double, std::milli>(end - start).count();
            fastSteps += steps;
            stepped = true;
        };
        if (singleStep)
            simulateStep();
        if (running)
        {
            accumulator += elapsed / speedDivisor; //шаг физики остаётся тем же
            while (accumulator >= dt)
            {
                simulateStep();
                accumulator -= dt;
            }
        }
        if (running || singleStep)
        {
            stats.totalPhysicsMs += physicsMs;
            ++stats.frames;
            // Пустой кадр учитывается в среднем, но не затирает последний замер на экране.
            if (stepped)
            {
                stats.fastSteps = fastSteps;
                lastMeasured = stats;
            }
        }
        drawWorld(window, font, hasFont, world, previous, mode, running, speedDivisor,
            running ? accumulator / dt : 1.0, lastMeasured);
        window.display();
    }
}
