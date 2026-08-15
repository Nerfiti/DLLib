#pragma once

#include <atomic>
#include <condition_variable>
#include <functional>
#include <mutex>

#include <SFML/Graphics/Color.hpp>

struct DrawState final
{
    sf::Color fillColor = sf::Color::Black;
    sf::Color color = sf::Color::White;
    float thickness = 1.f;
};

class CommandManager final
{
public:

    using cmd_t = std::function<void(DrawState&)>;

public:

    void addCommand (cmd_t&& command);
    void sendCommands ();
    void executeCommands (DrawState& drawState);

private:

    std::vector<cmd_t> commands_;

    //TODO: lock-free commands
    std::atomic<bool> commandListFull_ = false;
    std::mutex commandListMutex_;
    std::condition_variable commandListReadyToWrite_;
};