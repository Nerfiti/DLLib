#pragma once


#include <cstdint>
#include <memory>
#include <string_view>

#include "SFML/Graphics.hpp"


struct WindowContext;
using window_handler_t = std::weak_ptr<WindowContext>;


window_handler_t getLastWindow ();

window_handler_t createWindow (uint32_t width, uint32_t height, std::string_view title = "", sf::State state = sf::State::Windowed);
void display ();

void setFillColor (sf::Color color);
void setColor (sf::Color color);
void setThinkness (float thickness);

void clear (window_handler_t windowHandler = getLastWindow());
void drawLine (int x0, int y0, int x1, int y1, window_handler_t windowHandler = getLastWindow());
void drawCircle (int centerX, int centerY, float radius, window_handler_t windowHandler = getLastWindow());
void drawEllipse (int x0, int y0, int x1, int y1, window_handler_t windowHandler = getLastWindow());
void drawRect (int x0, int y0, int x1, int y1, window_handler_t windowHandler = getLastWindow());
void drawPolygon (sf::Vector2f points[], uint32_t numPoints, window_handler_t windowHandler = getLastWindow());
void drawTriangle (int x0, int y0, int x1, int y1, int x2, int y2, window_handler_t windowHandler = getLastWindow());

void setPixel (int x, int y, window_handler_t windowHandler = getLastWindow());
sf::Color getPixel (int x, int y, window_handler_t windowHandler = getLastWindow());


int _main ();
#define main () _main ()
