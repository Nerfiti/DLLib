#include <iostream>

#include <atomic>
#include <condition_variable>
#include <functional>
#include <future>
#include <list>
#include <memory>
#include <thread>
#include <vector>

#include <SFML/Graphics.hpp>

namespace detail
{

namespace windows
{
    struct WindowContext final
    {
        sf::RenderWindow window;
        sf::RenderTexture texture;

        WindowContext(uint32_t width, uint32_t height, std::string_view title, sf::State state)
            : window(sf::VideoMode({width, height}), title, state)
            , texture({width, height})
            {}
    };

    std::list<WindowContext> _window_contexts;
    using window_handler_t = decltype(_window_contexts.begin());

    window_handler_t createWindow(uint32_t width, uint32_t height, std::string_view title, sf::State state)
    {
        _window_contexts.emplace_front(width, height, title, state);
        return _window_contexts.begin();
    }

    window_handler_t getLastWindow()
    {
        return _window_contexts.begin();
    }

    void removeClosed()
    {
        auto it = _window_contexts.begin();

        while (it != _window_contexts.end())
        {
            if (!it->window.isOpen())
                it = _window_contexts.erase(it);
            else
                ++it;
        }
    }

    void pollEvents()
    {
        for (auto& window_ctx : _window_contexts)
        {
            while (const std::optional event = window_ctx.window.pollEvent())
                if (event->is<sf::Event::Closed>())
                    window_ctx.window.close();
        }
    }

    void displayWindow(WindowContext& window_ctx)
    {
        window_ctx.texture.display();
        sf::Sprite texture_sprite(window_ctx.texture.getTexture());

        window_ctx.window.clear(sf::Color::Magenta);
        window_ctx.window.draw(texture_sprite);
        window_ctx.window.display();
    }

    void displayWindows()
    {
        for (auto& window_ctx : _window_contexts)
            displayWindow(window_ctx);
    }
}

namespace commands
{
    std::thread _user_thread;

    struct DrawState final
    {
        sf::Color fillColor = sf::Color::Transparent;
        sf::Color сolor = sf::Color::Transparent;
        
        float thinkness;
    };

    using cmd_t = std::function<void(DrawState&)>;
    std::vector<cmd_t> _commands;

    //TODO: lock-free commands
    std::atomic<bool> _command_list_full = false;
    std::mutex _command_list_mutex;
    std::condition_variable _command_list_ready_to_write;

    void addCommand(cmd_t&& command)
    {
        std::unique_lock lock(_command_list_mutex);

        _command_list_ready_to_write.wait(lock, [] { return !_command_list_full; });

        _commands.emplace_back(std::move(command));
    }

    void sendCommands()
    {
        _command_list_full.store(true);
    }

    void executeCommands(DrawState& draw_state)
    {
        if (_command_list_full.load() == false)
            return;

        std::vector<cmd_t> cmds;
        {
            std::lock_guard guard(_command_list_mutex);
            std::swap(cmds, _commands);
        }

        _command_list_full.store(false);
        _command_list_ready_to_write.notify_all();
        

        for (auto& command : cmds)
        {
            command(draw_state);
        }
    }
};

};

//---------------------------------------------------------

using detail::windows::window_handler_t;

void display()
{
    detail::commands::addCommand([] (detail::commands::DrawState&)
    {
        detail::windows::displayWindows();
    });
    detail::commands::sendCommands();
}

window_handler_t createWindow(uint32_t width, uint32_t height, std::string_view title = "", sf::State state = sf::State::Windowed)
{
    auto promise = std::make_shared<std::promise<window_handler_t>>();
    auto future = promise->get_future();

    detail::commands::addCommand([=] (detail::commands::DrawState&) mutable
    {
        window_handler_t wh = detail::windows::createWindow(width, height, title, state);
        detail::windows::displayWindow(*wh);

        promise->set_value(wh);
    });

    detail::commands::sendCommands();
    return future.get();
}

//---------------------------------------------------------

void setFillColor(sf::Color color)
{
    detail::commands::addCommand([=] (detail::commands::DrawState& state)
    {
        state.fillColor = color;
    });
}

void setColor(sf::Color color)
{
    detail::commands::addCommand([=] (detail::commands::DrawState& state)
    {
        state.сolor = color;
    });
}

void setThinkness(float thinkness)
{
    detail::commands::addCommand([=] (detail::commands::DrawState& state)
    {
        state.thinkness = thinkness;
    });
}

void clear(window_handler_t window_handler = detail::windows::getLastWindow())
{
    detail::commands::addCommand([=] (detail::commands::DrawState& state)
    {
        window_handler->texture.clear(state.fillColor);
    });
}

void drawLine(int x0, int y0, int x1, int y1, window_handler_t window_handler = detail::windows::getLastWindow())
{
    if (x1 < x0)
    {
        std::swap(x0, x1);
        std::swap(y0, y1);
    }

    int64_t dx = x1 - x0;
    int64_t dy = y1 - y0;
    float len = sqrt(dx * dx + dy * dy);

    detail::commands::addCommand([=] (detail::commands::DrawState& state)
    {
        sf::RectangleShape line(sf::Vector2f(len, state.thinkness));
        line.setFillColor(state.fillColor);

        line.setPosition(sf::Vector2f(x0, y0));
        line.rotate(sf::radians(std::atan(static_cast<float>(dy)/dx)));

        window_handler->texture.draw(line);
    });
}

void drawCircle(int center_x, int center_y, float radius, window_handler_t window_handler = detail::windows::getLastWindow())
{
    detail::commands::addCommand([=] (detail::commands::DrawState& state)
    {
        sf::CircleShape circle(radius);
        circle.setFillColor(state.fillColor);
        circle.setOutlineColor(state.сolor);
        circle.setOutlineThickness(state.thinkness);

        circle.setOrigin(sf::Vector2f(radius, radius));
        circle.setPosition(sf::Vector2f(center_x, center_y));

        window_handler->texture.draw(circle);
    });
}

void drawEllipse(int x0, int y0, int x1, int y1, window_handler_t window_handler = detail::windows::getLastWindow())
{
    if (x1 < x0)
    {
        std::swap(x0, x1);
        std::swap(y0, y1);
    }

    int64_t width = x1 - x0 + 1;
    int64_t height = y1 - y0 + 1;

    detail::commands::addCommand([=] (detail::commands::DrawState& state)
    {
        sf::CircleShape ellipse(height);
        ellipse.setFillColor(state.fillColor);
        ellipse.setOutlineColor(state.сolor);
        ellipse.setOutlineThickness(state.thinkness);

        ellipse.scale(sf::Vector2f(static_cast<float>(width) / height, 1.0f));
        ellipse.setPosition(sf::Vector2f(x0, y0));

        window_handler->texture.draw(ellipse);
    });
}

void drawRect(int x0, int y0, int x1, int y1, window_handler_t window_handler = detail::windows::getLastWindow())
{
    if (x1 < x0)
    {
        std::swap(x0, x1);
        std::swap(y0, y1);
    }

    int64_t width = x1 - x0 + 1;
    int64_t height = y1 - y0 + 1;

    detail::commands::addCommand([=] (detail::commands::DrawState& state)
    {
        sf::RectangleShape rect(sf::Vector2f(width, height));
        rect.setFillColor(state.fillColor);
        rect.setOutlineColor(state.сolor);
        rect.setOutlineThickness(state.thinkness);

        rect.setPosition(sf::Vector2f(x0, y0));

        window_handler->texture.draw(rect);
    });
}

void drawPolygon(sf::Vector2f points[], uint32_t num_points, window_handler_t window_handler = detail::windows::getLastWindow())
{
    sf::ConvexShape polygon(num_points);
    for (size_t i = 0; i < num_points; ++i)
        polygon.setPoint(i, points[i]);

    detail::commands::addCommand([=] (detail::commands::DrawState& state) mutable
    {
        polygon.setFillColor(state.fillColor);
        polygon.setOutlineColor(state.сolor);
        polygon.setOutlineThickness(state.thinkness);

        window_handler->texture.draw(polygon);
    });
}

void drawTriangle(int x0, int y0, int x1, int y1, int x2, int y2, window_handler_t window_handler = detail::windows::getLastWindow())
{
    constexpr size_t kNumPoints = 3;
    sf::Vector2f points[kNumPoints] = {sf::Vector2f(x0, y0), sf::Vector2f(x1, y1), sf::Vector2f(x2, y2)};

    drawPolygon(points, kNumPoints, window_handler);
}

//TODO: Ensure that float conversion does not break the operation
void setPixel(int x, int y, window_handler_t window_handler = detail::windows::getLastWindow())
{
    drawRect(x, y, x, y, window_handler);
}

sf::Color getPixel(int x, int y, window_handler_t window_handler = detail::windows::getLastWindow())
{
    auto promise = std::make_shared<std::promise<sf::Color>>();
    auto future = promise->get_future();

    detail::commands::addCommand([=] (detail::commands::DrawState&) mutable
    {
        sf::Color pixel = window_handler->texture.getTexture().copyToImage().getPixel(sf::Vector2u(x, y));
        promise->set_value(pixel);
    });

    detail::commands::sendCommands();
    return future.get();
}

//---------------------------------------------------------

#define USER_THREAD

int command_thread();

#ifdef USER_THREAD
int main()
{
    static std::atomic<bool> user_thread_finished = false;

    detail::commands::_user_thread = std::thread([] {
        command_thread();
        detail::commands::sendCommands();

        user_thread_finished.store(true);
    });

    detail::commands::DrawState draw_state = {};
    while(true)
    {
        if (user_thread_finished && detail::windows::_window_contexts.empty())
            break;

        detail::windows::removeClosed();
        detail::windows::pollEvents();

        detail::commands::executeCommands(draw_state);
    }

    if (detail::commands::_user_thread.joinable())
        detail::commands::_user_thread.join();
}

#define main() command_thread()
#endif
