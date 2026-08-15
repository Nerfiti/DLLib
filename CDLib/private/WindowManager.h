#pragma once

#include <list>

#include <SFML/Graphics/RenderTexture.hpp>
#include <SFML/Graphics/RenderWindow.hpp>

struct WindowContext final
{
    sf::RenderWindow window;
    sf::RenderTexture texture;

    WindowContext(uint32_t width, uint32_t height, std::string_view title, sf::State state);
};

class WindowManager final
{
public:

    std::weak_ptr<WindowContext> createWindow(uint32_t width, uint32_t height, std::string_view title, sf::State state);
    std::weak_ptr<WindowContext> getLastWindow();
    void removeClosed();
    void pollEvents();
    void displayWindow(std::weak_ptr<WindowContext> windowHandler);
    void displayWindows();
    bool hasWindows();

private:
    
    std::list<std::shared_ptr<WindowContext>> windowContexts_;

};