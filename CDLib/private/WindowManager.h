#pragma once


#include <utils/Storage.h>

#include <SFML/Graphics/RenderTexture.hpp>
#include <SFML/Graphics/RenderWindow.hpp>

class WindowManager final
{   
public:

    struct Window final
    {
        sf::RenderWindow sfWindow;
        sf::RenderTexture sfTexture;
        
        Window (uint32_t width, uint32_t height, std::string_view title, sf::State state);
    };
    using storage_t = utils::Storage<Window>;
    using WindowHandler = storage_t::Key;

public:

    WindowHandler createWindow (uint32_t width, uint32_t height, std::string_view title, sf::State state);
    WindowHandler getLastWindow ();
    void removeClosed ();
    void pollEvents ();
    void displayWindow (WindowHandler windowHandler);
    void displayWindows ();
    bool hasWindows ();

    Window& getWindow (WindowHandler windowHandler);
    const Window& getWindow (WindowHandler windowHandler) const;

private:

    void displayWindow(Window& window);
    
    storage_t windows_;
    WindowHandler lastWindowHandler_ = InvalidWindowHandler;

private:

    static const WindowHandler InvalidWindowHandler;
};