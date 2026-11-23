#include "display.h"

#include <string_view>

int main(int argc, char** argv)
{
    const std::array results{ runExperiment(Mode::Discrete), runExperiment(Mode::Ccd),
        runExperiment(Mode::MultiRate) };

    if (argc == 2 && std::string_view(argv[1]) == "--check")
        return checkExperiment(results) ? 0 : 1;

    sf::RenderWindow window(sf::VideoMode({ 1020, 528 }), L"Столкновение со стенкой",
        sf::Style::Titlebar | sf::Style::Close);
    window.setFramerateLimit(60);
    sf::Font font;
    const bool hasFont = font.openFromFile("C:/Windows/Fonts/arial.ttf");

    bool showResult = false;
    int visibleMode = -1;

    while (window.isOpen())
    {
        while (const auto event = window.pollEvent())
        {
            if (event->is<sf::Event::Closed>())
                window.close();
            if (const auto* key = event->getIf<sf::Event::KeyPressed>())
            {
                using Key = sf::Keyboard::Key;
                if (key->code == Key::Escape) window.close();
                if (key->code == Key::Space) showResult = true;
                if (key->code == Key::R) showResult = false;
                if (key->code == Key::Num0) visibleMode = -1;
                if (key->code == Key::Num1) visibleMode = 0;
                if (key->code == Key::Num2) visibleMode = 1;
                if (key->code == Key::Num3) visibleMode = 2;
            }
        }
        if (!window.isOpen())
            break;

        drawExperiment(window, font, hasFont, results, showResult, visibleMode);
        window.display();
    }
}
