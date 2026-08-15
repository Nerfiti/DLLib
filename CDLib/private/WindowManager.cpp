#include "WindowManager.h"

#include <SFML/Graphics/Sprite.hpp>


WindowContext::WindowContext (uint32_t width, uint32_t height, std::string_view title, sf::State state):
  window(sf::VideoMode({width, height}), std::string(title), state), 
  texture({width, height}) 
{}

std::weak_ptr<WindowContext> WindowManager::createWindow (uint32_t width, uint32_t height, std::string_view title, sf::State state)
{
    auto ctx = std::make_shared<WindowContext>(width, height, title, state);
    return windowContexts_.emplace_front(std::move(ctx));
}

std::weak_ptr<WindowContext> WindowManager::getLastWindow ()
{
    return windowContexts_.front();
}

void WindowManager::removeClosed ()
{
    auto it = windowContexts_.begin();

    while (it != windowContexts_.end())
    {
        if (!(*it)->window.isOpen())
            it = windowContexts_.erase(it);
        else
            ++it;
    }
}

void WindowManager::pollEvents ()
{
    for (auto& windowCtx : windowContexts_)
    {
        while (const std::optional event = windowCtx->window.pollEvent())
            if (event->is<sf::Event::Closed>())
                windowCtx->window.close();
    }
}

void WindowManager::displayWindow (std::weak_ptr<WindowContext> windowHandler)
{
    if (windowHandler.expired())
        return;
    
    auto windowCtx = windowHandler.lock();

    windowCtx->texture.display();
    sf::Sprite textureSprite(windowCtx->texture.getTexture());

    windowCtx->window.clear(sf::Color::Magenta);
    windowCtx->window.draw(textureSprite);
    windowCtx->window.display();
}

void WindowManager::displayWindows ()
{
    for (auto& windowCtx : windowContexts_)
        displayWindow(windowCtx);
}

bool WindowManager::hasWindows ()
{
    return !windowContexts_.empty();
}