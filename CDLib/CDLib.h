#pragma once

#include <iostream>

#include <atomic>
#include <cmath>
#include <condition_variable>
#include <cstdint>
#include <functional>
#include <future>
#include <list>
#include <memory>
#include <string>
#include <string_view>
#include <thread>
#include <vector>

#include "SFML/Graphics.hpp"

namespace detail
{

struct DrawState final
{
    sf::Color fillColor = sf::Color::Black;
    sf::Color color = sf::Color::White;
    float thickness = 1.f;
};

class WindowManager final
{
public:

    struct WindowContext final
    {
        sf::RenderWindow window;
        sf::RenderTexture texture;

        WindowContext(uint32_t width, uint32_t height, std::string_view title, sf::State state);
    };

private:

    std::list<WindowContext> windowContexts_;

public:

    using window_handler_t = decltype(windowContexts_.begin());

    window_handler_t createWindow(uint32_t width, uint32_t height, std::string_view title, sf::State state);
    window_handler_t getLastWindow();
    void removeClosed();
    void pollEvents();
    void displayWindow(WindowContext& windowCtx);
    void displayWindows();
    bool hasWindows();

} g_WindowManager;

class CommandManager final
{
public:

    using cmd_t = std::function<void(DrawState&)>;

public:

    void addCommand(cmd_t&& command);
    void sendCommands();
    void executeCommands(DrawState& drawState);

private:

    std::vector<cmd_t> commands_;

    //TODO: lock-free commands
    std::atomic<bool> commandListFull_ = false;
    std::mutex commandListMutex_;
    std::condition_variable commandListReadyToWrite_;
} g_CommandManager;

};

//---------------------------------------------------------

using window_handler_t = detail::WindowManager::window_handler_t;

window_handler_t createWindow(uint32_t width, uint32_t height, std::string_view title = "", sf::State state = sf::State::Windowed);
void display();

void setFillColor(sf::Color color);
void setColor(sf::Color color);
void setThinkness(float thickness);

void clear(window_handler_t windowHandler = detail::g_WindowManager.getLastWindow());
void drawLine(int x0, int y0, int x1, int y1, window_handler_t windowHandler = detail::g_WindowManager.getLastWindow());
void drawCircle(int centerX, int centerY, float radius, window_handler_t windowHandler = detail::g_WindowManager.getLastWindow());
void drawEllipse(int x0, int y0, int x1, int y1, window_handler_t windowHandler = detail::g_WindowManager.getLastWindow());
void drawRect(int x0, int y0, int x1, int y1, window_handler_t windowHandler = detail::g_WindowManager.getLastWindow());
void drawPolygon(sf::Vector2f points[], uint32_t numPoints, window_handler_t windowHandler = detail::g_WindowManager.getLastWindow());
void drawTriangle(int x0, int y0, int x1, int y1, int x2, int y2, window_handler_t windowHandler = detail::g_WindowManager.getLastWindow());

void setPixel(int x, int y, window_handler_t windowHandler = detail::g_WindowManager.getLastWindow());
sf::Color getPixel(int x, int y, window_handler_t windowHandler = detail::g_WindowManager.getLastWindow());

//---------------------------------------------------------

namespace detail
{

WindowManager::WindowContext::WindowContext(uint32_t width, uint32_t height, std::string_view title, sf::State state)
    : window(sf::VideoMode({width, height}), std::string(title), state)
    , texture({width, height})
    {}

WindowManager::window_handler_t WindowManager::createWindow(uint32_t width, uint32_t height, std::string_view title, sf::State state)
{
    windowContexts_.emplace_front(width, height, title, state);
    return windowContexts_.begin();
}

WindowManager::window_handler_t WindowManager::getLastWindow()
{
    return windowContexts_.begin();
}

void WindowManager::removeClosed()
{
    auto it = windowContexts_.begin();

    while (it != windowContexts_.end())
    {
        if (!it->window.isOpen())
            it = windowContexts_.erase(it);
        else
            ++it;
    }
}

void WindowManager::pollEvents()
{
    for (auto& windowCtx : windowContexts_)
    {
        while (const std::optional event = windowCtx.window.pollEvent())
            if (event->is<sf::Event::Closed>())
                windowCtx.window.close();
    }
}

void WindowManager::displayWindow(WindowContext& windowCtx)
{
    windowCtx.texture.display();
    sf::Sprite textureSprite(windowCtx.texture.getTexture());

    windowCtx.window.clear(sf::Color::Magenta);
    windowCtx.window.draw(textureSprite);
    windowCtx.window.display();
}

void WindowManager::displayWindows()
{
    for (auto& windowCtx : windowContexts_)
        displayWindow(windowCtx);
}

bool WindowManager::hasWindows()
{
    return !windowContexts_.empty();
}

//---------------------------------------------------------

void CommandManager::addCommand(cmd_t&& command)
{
    std::unique_lock lock(commandListMutex_);

    commandListReadyToWrite_.wait(lock, [this] { return !commandListFull_; });

    commands_.emplace_back(std::move(command));
}

void CommandManager::sendCommands()
{
    commandListFull_.store(true);
}

void CommandManager::executeCommands(DrawState& drawState)
{
    if (commandListFull_.load() == false)
        return;

    std::vector<cmd_t> cmds;
    {
        std::lock_guard guard(commandListMutex_);
        std::swap(cmds, commands_);
    }

    commandListFull_.store(false);
    commandListReadyToWrite_.notify_all();
    

    for (auto& command : cmds)
    {
        command(drawState);
    }
}

} //namespace detail

//---------------------------------------------------------

window_handler_t createWindow(uint32_t width, uint32_t height, std::string_view title, sf::State state)
{
    auto promise = std::make_shared<std::promise<window_handler_t>>();
    auto future = promise->get_future();

    detail::g_CommandManager.addCommand([=] (detail::DrawState& drawState) mutable
    {
        window_handler_t wh = detail::g_WindowManager.createWindow(width, height, title, state);

        wh->texture.clear(drawState.fillColor);
        detail::g_WindowManager.displayWindow(*wh);

        promise->set_value(wh);
    });

    detail::g_CommandManager.sendCommands();
    return future.get();
}

void display()
{
    detail::g_CommandManager.addCommand([] (detail::DrawState&)
    {
        detail::g_WindowManager.displayWindows();
    });
    detail::g_CommandManager.sendCommands();
}

void setFillColor(sf::Color color)
{
    detail::g_CommandManager.addCommand([=] (detail::DrawState& state)
    {
        state.fillColor = color;
    });
}

void setColor(sf::Color color)
{
    detail::g_CommandManager.addCommand([=] (detail::DrawState& state)
    {
        state.color = color;
    });
}

void setThinkness(float thickness)
{
    detail::g_CommandManager.addCommand([=] (detail::DrawState& state)
    {
        state.thickness = thickness;
    });
}

void clear(window_handler_t windowHandler)
{
    detail::g_CommandManager.addCommand([=] (detail::DrawState& state)
    {
        windowHandler->texture.clear(state.fillColor);
    });
}

void drawLine(int x0, int y0, int x1, int y1, window_handler_t windowHandler)
{
    if (x1 < x0)
    {
        std::swap(x0, x1);
        std::swap(y0, y1);
    }

    int64_t dx = x1 - x0;
    int64_t dy = y1 - y0;
    float len = sqrt(dx * dx + dy * dy);

    detail::g_CommandManager.addCommand([=] (detail::DrawState& state)
    {
        sf::RectangleShape line(sf::Vector2f(len, state.thickness));
        line.setFillColor(state.color);

        line.setPosition(sf::Vector2f(x0, y0));
        line.rotate(sf::radians(std::atan(static_cast<float>(dy)/dx)));

        windowHandler->texture.draw(line);
    });
}

void drawCircle(int centerX, int centerY, float radius, window_handler_t windowHandler)
{
    detail::g_CommandManager.addCommand([=] (detail::DrawState& state)
    {
        sf::CircleShape circle(radius);
        circle.setFillColor(state.fillColor);
        circle.setOutlineColor(state.color);
        circle.setOutlineThickness(state.thickness);

        circle.setOrigin(sf::Vector2f(radius, radius));
        circle.setPosition(sf::Vector2f(centerX, centerY));

        windowHandler->texture.draw(circle);
    });
}

void drawEllipse(int x0, int y0, int x1, int y1, window_handler_t windowHandler)
{
    if (x1 < x0)
        std::swap(x0, x1);
    if (y1 < y0)
        std::swap(y0, y1);

    int64_t width = x1 - x0 + 1;
    int64_t height = y1 - y0 + 1;

    float radius = height / 2.f;
    sf::Vector2f scaleFactor(static_cast<float>(width) / height, 1.f);
    if (width < height)
    {
        radius = height;
        scaleFactor = sf::Vector2f(1.f, static_cast<float>(height) / width);
    }

    detail::g_CommandManager.addCommand([=] (detail::DrawState& state)
    {
        constexpr size_t kNumPoints = 60;
        sf::CircleShape ellipse(radius, kNumPoints);
        ellipse.setFillColor(state.fillColor);
        ellipse.setOutlineColor(state.color);
        ellipse.setOutlineThickness(state.thickness);

        ellipse.scale(scaleFactor);
        ellipse.setPosition(sf::Vector2f(x0, y0));

        windowHandler->texture.draw(ellipse);
    });
}

void drawRect(int x0, int y0, int x1, int y1, window_handler_t windowHandler)
{
    if (x1 < x0)
        std::swap(x0, x1);
    if (y1 < y0)
        std::swap(y0, y1);

    int64_t width = x1 - x0 + 1;
    int64_t height = y1 - y0 + 1;

    detail::g_CommandManager.addCommand([=] (detail::DrawState& state)
    {
        sf::RectangleShape rect(sf::Vector2f(width, height));
        rect.setFillColor(state.fillColor);
        rect.setOutlineColor(state.color);
        rect.setOutlineThickness(state.thickness);

        rect.setPosition(sf::Vector2f(x0, y0));

        windowHandler->texture.draw(rect);
    });
}

void drawPolygon(sf::Vector2f points[], uint32_t numPoints, window_handler_t windowHandler)
{
    sf::ConvexShape polygon(numPoints);
    for (size_t i = 0; i < numPoints; ++i)
        polygon.setPoint(i, points[i]);

    detail::g_CommandManager.addCommand([=] (detail::DrawState& state) mutable
    {
        polygon.setFillColor(state.fillColor);
        polygon.setOutlineColor(state.color);
        polygon.setOutlineThickness(state.thickness);

        windowHandler->texture.draw(polygon);
    });
}

void drawTriangle(int x0, int y0, int x1, int y1, int x2, int y2, window_handler_t windowHandler)
{
    constexpr size_t kNumPoints = 3;
    sf::Vector2f points[kNumPoints] = {sf::Vector2f(x0, y0), sf::Vector2f(x1, y1), sf::Vector2f(x2, y2)};

    drawPolygon(points, kNumPoints, windowHandler);
}

//TODO: Ensure that float conversion does not break the operation
void setPixel(int x, int y, window_handler_t windowHandler)
{
    drawRect(x, y, x, y, windowHandler);
}

sf::Color getPixel(int x, int y, window_handler_t windowHandler)
{
    auto promise = std::make_shared<std::promise<sf::Color>>();
    auto future = promise->get_future();

    detail::g_CommandManager.addCommand([=] (detail::DrawState&) mutable
    {
        sf::Color pixel = windowHandler->texture.getTexture().copyToImage().getPixel(sf::Vector2u(x, y));
        promise->set_value(pixel);
    });

    detail::g_CommandManager.sendCommands();
    return future.get();
}

//---------------------------------------------------------

int command_thread();

int main()
{
    static std::atomic<bool> user_thread_finished = false;

    std::thread user_thread = std::thread([] {
        command_thread();
        display();

        user_thread_finished.store(true);
    });

    detail::DrawState drawState = {};
    while(true)
    {
        if (user_thread_finished && !detail::g_WindowManager.hasWindows())
            break;

        detail::g_WindowManager.removeClosed();
        detail::g_WindowManager.pollEvents();

        detail::g_CommandManager.executeCommands(drawState);
    }

    if (user_thread.joinable())
        user_thread.join();
}

#define main() command_thread()
