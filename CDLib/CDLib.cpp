#include "CDLib.h"
#undef main

#include "private/CommandManager.h"
#include "private/WindowManager.h"

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


static struct GlobalContext
{
    WindowManager windowManager;
    CommandManager commandManager;
} g_Ctx;


window_handler_t getLastWindow ()
{
    return g_Ctx.windowManager.getLastWindow();
}

window_handler_t createWindow (uint32_t width, uint32_t height, std::string_view title, sf::State state)
{
    auto promise = std::make_shared<std::promise<window_handler_t>>();
    auto future = promise->get_future();

    g_Ctx.commandManager.addCommand([=] (DrawState& drawState) mutable
    {
        auto wh = g_Ctx.windowManager.createWindow(width, height, title, state).lock();

        wh->texture.clear(drawState.fillColor);
        g_Ctx.windowManager.displayWindow(wh);

        promise->set_value(wh);
    });

    g_Ctx.commandManager.sendCommands();
    return future.get();
}

void display ()
{
    g_Ctx.commandManager.addCommand([] (DrawState&)
    {
        g_Ctx.windowManager.displayWindows();
    });
    g_Ctx.commandManager.sendCommands();
}

void setFillColor (sf::Color color)
{
    g_Ctx.commandManager.addCommand([=] (DrawState& state)
    {
        state.fillColor = color;
    });
}

void setColor (sf::Color color)
{
    g_Ctx.commandManager.addCommand([=] (DrawState& state)
    {
        state.color = color;
    });
}

void setThinkness (float thickness)
{
    g_Ctx.commandManager.addCommand([=] (DrawState& state)
    {
        state.thickness = thickness;
    });
}

void clear (window_handler_t windowHandler)
{
    g_Ctx.commandManager.addCommand([=] (DrawState& state)
    {
        if (windowHandler.expired())
            return;

        auto windowCtx = windowHandler.lock();
        
        windowCtx->texture.clear(state.fillColor);
    });
}

void drawLine (int x0, int y0, int x1, int y1, window_handler_t windowHandler)
{
    g_Ctx.commandManager.addCommand([=] (DrawState& state) mutable
    {
        if (windowHandler.expired())
            return;

        auto windowCtx = windowHandler.lock();

        if (x1 < x0)
        {
            std::swap(x0, x1);
            std::swap(y0, y1);
        }

        int64_t dx = x1 - x0;
        int64_t dy = y1 - y0;
        float len = sqrt(dx * dx + dy * dy);

        sf::RectangleShape line(sf::Vector2f(len, state.thickness));
        line.setFillColor(state.color);

        line.setPosition(sf::Vector2f(x0, y0));
        line.rotate(sf::radians(std::atan(static_cast<float>(dy)/dx)));

        windowCtx->texture.draw(line);
    });
}

void drawCircle (int centerX, int centerY, float radius, window_handler_t windowHandler)
{
    g_Ctx.commandManager.addCommand([=] (DrawState& state)
    {
        if (windowHandler.expired())
            return;

        auto windowCtx = windowHandler.lock();

        sf::CircleShape circle(radius);
        circle.setFillColor(state.fillColor);
        circle.setOutlineColor(state.color);
        circle.setOutlineThickness(state.thickness);

        circle.setOrigin(sf::Vector2f(radius, radius));
        circle.setPosition(sf::Vector2f(centerX, centerY));

        windowCtx->texture.draw(circle);
    });
}

void drawEllipse (int x0, int y0, int x1, int y1, window_handler_t windowHandler)
{
    g_Ctx.commandManager.addCommand([=] (DrawState& state) mutable
    {
        if (windowHandler.expired())
            return;

        auto windowCtx = windowHandler.lock();

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
            radius = width / 2.f;
            scaleFactor = sf::Vector2f(1.f, static_cast<float>(height) / width);
        }

        sf::CircleShape ellipse(radius);
        ellipse.setFillColor(state.fillColor);
        ellipse.setOutlineColor(state.color);
        ellipse.setOutlineThickness(state.thickness);

        ellipse.scale(scaleFactor);
        ellipse.setPosition(sf::Vector2f(x0, y0));

        windowCtx->texture.draw(ellipse);
    });
}

void drawRect (int x0, int y0, int x1, int y1, window_handler_t windowHandler)
{
    g_Ctx.commandManager.addCommand([=] (DrawState& state) mutable
    {
        if (windowHandler.expired())
            return;

        auto windowCtx = windowHandler.lock();

        if (x1 < x0)
            std::swap(x0, x1);
        if (y1 < y0)
            std::swap(y0, y1);

        int64_t width = x1 - x0 + 1;
        int64_t height = y1 - y0 + 1;

        sf::RectangleShape rect(sf::Vector2f(width, height));
        rect.setFillColor(state.fillColor);
        rect.setOutlineColor(state.color);
        rect.setOutlineThickness(state.thickness);

        rect.setPosition(sf::Vector2f(x0, y0));

        windowCtx->texture.draw(rect);
    });
}

void drawPolygon (sf::Vector2f points[], uint32_t numPoints, window_handler_t windowHandler)
{
    sf::ConvexShape polygon(numPoints);
    for (size_t i = 0; i < numPoints; ++i)
        polygon.setPoint(i, points[i]);

    g_Ctx.commandManager.addCommand([=] (DrawState& state) mutable
    {
        if (windowHandler.expired())
            return;

        auto windowCtx = windowHandler.lock();

        polygon.setFillColor(state.fillColor);
        polygon.setOutlineColor(state.color);
        polygon.setOutlineThickness(state.thickness);

        windowCtx->texture.draw(polygon);
    });
}

void drawTriangle (int x0, int y0, int x1, int y1, int x2, int y2, window_handler_t windowHandler)
{
    constexpr size_t kNumPoints = 3;
    sf::Vector2f points[kNumPoints] = {sf::Vector2f(x0, y0), sf::Vector2f(x1, y1), sf::Vector2f(x2, y2)};

    drawPolygon(points, kNumPoints, windowHandler);
}

void setPixel(int x, int y, window_handler_t windowHandler)
{
    drawRect(x, y, x, y, windowHandler);
}

sf::Color getPixel (int x, int y, window_handler_t windowHandler)
{
    auto promise = std::make_shared<std::promise<sf::Color>>();
    auto future = promise->get_future();

    g_Ctx.commandManager.addCommand([=] (DrawState&) mutable
    {
        if (windowHandler.expired())
            return;

        auto windowCtx = windowHandler.lock();

        sf::Color pixel = windowCtx->texture.getTexture().copyToImage().getPixel(sf::Vector2u(x, y));
        promise->set_value(pixel);
    });

    g_Ctx.commandManager.sendCommands();
    return future.get();
}


int main ()
{
    std::atomic<bool> user_thread_finished = false;

    std::thread user_thread = std::thread([&] {
        _main();
        display();

        user_thread_finished.store(true);
    });

    DrawState drawState = {};
    while(true)
    {
        if (user_thread_finished && !g_Ctx.windowManager.hasWindows())
            break;

        g_Ctx.windowManager.removeClosed();
        g_Ctx.windowManager.pollEvents();

        g_Ctx.commandManager.executeCommands(drawState);
    }

    if (user_thread.joinable())
        user_thread.join();
}