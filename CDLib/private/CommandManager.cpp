#include "CommandManager.h"

void CommandManager::addCommand (cmd_t&& command)
{
    std::unique_lock lock(commandListMutex_);

    commandListReadyToWrite_.wait(lock, [this] { return !commandListFull_; });

    commands_.emplace_back(std::move(command));
}

void CommandManager::sendCommands ()
{
    commandListFull_.store(true);
}

void CommandManager::executeCommands (DrawState& drawState)
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