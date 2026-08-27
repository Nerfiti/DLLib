#include "WindowManager.h"
#include "CDLib.h"

#include <SFML/Graphics/Sprite.hpp>
#include <vector>

#include <limits>
const WindowManager::WindowHandler WindowManager::InvalidWindowHandler = std::numeric_limits<WindowManager::WindowHandler>::max();


WindowManager::Window::Window (uint32_t width, uint32_t height, std::string_view title, sf::State state):
  sfWindow(sf::VideoMode({width, height}), std::string(title), state), 
  sfTexture({width, height}) 
{}

WindowManager::WindowHandler WindowManager::createWindow (uint32_t width, uint32_t height, std::string_view title, sf::State state)
{
    return lastWindowHandler_ = windows_.emplace(width, height, title, state);
}

WindowManager::WindowHandler WindowManager::getLastWindow ()
{
    if (lastWindowHandler_ == InvalidWindowHandler)
    {
        throw std::runtime_error("No windows created");
    }

    return lastWindowHandler_;
}

void WindowManager::removeClosed ()
{
    std::vector<WindowHandler> keysToRemove;

    for (const auto& window : windows_.getValues())
    {
        if (!window.value.sfWindow.isOpen())
            keysToRemove.push_back(window.key);
    }

    for (const WindowHandler& key : keysToRemove)
        windows_.remove(key);
}

void WindowManager::pollEvents ()
{
    for (auto& window : windows_.getValues())
    {
        while (const std::optional event = window.value.sfWindow.pollEvent())
        {
            if (event->is<sf::Event::Closed>())
                window.value.sfWindow.close();
        }
    }
}

void WindowManager::displayWindow (WindowHandler windowHandler)
{
    displayWindow(windows_.get(windowHandler));
}

void WindowManager::displayWindows ()
{
    for (auto& window : windows_.getValues())
        displayWindow(window.value);
}

bool WindowManager::hasWindows ()
{
    return !windows_.getValues().empty();
}

WindowManager::Window& WindowManager::getWindow (WindowManager::WindowHandler windowHandler)
{
    return windows_.get(windowHandler);
}

const WindowManager::Window& WindowManager::getWindow (WindowManager::WindowHandler windowHandler) const
{
    return windows_.get(windowHandler);
}


void WindowManager::displayWindow(Window& window)
{
    window.sfTexture.display();
    sf::Sprite textureSprite(window.sfTexture.getTexture());

    window.sfWindow.clear(sf::Color::Black);
    window.sfWindow.draw(textureSprite);
    window.sfWindow.display();
}