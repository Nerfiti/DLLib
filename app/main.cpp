#include <CDLib.h>
#include <iostream>


int main ()
{
    createWindow(550, 800, "CDLib");

    setColor(sf::Color::Magenta);
    setFillColor(sf::Color::Transparent);
    setThinkness(3.f);

    drawCircle(150, 650, 100);
    drawCircle(400, 650, 100);

    drawEllipse(175, 50, 375, 600);

    return 0;
}