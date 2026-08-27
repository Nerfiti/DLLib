#pragma once


#include <cstdint>
#include <memory>
#include <string_view>

#include "SFML/Graphics.hpp"

class WindowHandler
{
public:

    friend WindowHandler createWindow (uint32_t width, uint32_t height, std::string_view title, sf::State state);
    friend WindowHandler getLastWindow ();

    const uint64_t id;
    
    operator uint64_t() const { return id; }

private:

    WindowHandler (uint64_t id): id(id) {}
};


WindowHandler getLastWindow ();

WindowHandler createWindow (uint32_t width, uint32_t height, std::string_view title = "", sf::State state = sf::State::Windowed);
void display ();

void setFillColor (sf::Color color);
void setColor (sf::Color color);
void setThinkness (float thickness);

void clear (WindowHandler windowHandler = getLastWindow());
void drawLine (int x0, int y0, int x1, int y1, WindowHandler windowHandler = getLastWindow());
void drawCircle (int centerX, int centerY, float radius, WindowHandler windowHandler = getLastWindow());
void drawEllipse (int x0, int y0, int x1, int y1, WindowHandler windowHandler = getLastWindow());
void drawRect (int x0, int y0, int x1, int y1, WindowHandler windowHandler = getLastWindow());
void drawPolygon (sf::Vector2f points[], uint32_t numPoints, WindowHandler windowHandler = getLastWindow());
void drawTriangle (int x0, int y0, int x1, int y1, int x2, int y2, WindowHandler windowHandler = getLastWindow());

void setPixel (int x, int y, WindowHandler windowHandler = getLastWindow());
sf::Color getPixel (int x, int y, WindowHandler windowHandler = getLastWindow());


int _main ();
#define main() _main ()
