 #include <iostream>
 #include <cmath>
 
 #include <CDLib.h>

// ── helpers ──────────────────────────────────────────────────────────────────

static constexpr int   W   = 800;
static constexpr int   H   = 600;
static constexpr float PI  = 3.14159265f;
static constexpr float FPS = 2.f;

static sf::Color hueColor(float t)
{
    float r = 0.5f + 0.5f * std::sin(2.f * PI * t);
    float g = 0.5f + 0.5f * std::sin(2.f * PI * t + 2.f * PI / 3.f);
    float b = 0.5f + 0.5f * std::sin(2.f * PI * t + 4.f * PI / 3.f);
    return sf::Color(
        static_cast<uint8_t>(r * 255),
        static_cast<uint8_t>(g * 255),
        static_cast<uint8_t>(b * 255)
    );
}


// ── main ─────────────────────────────────────────────────────────────────────

int main()
{
    // ── 1. Window creation ────────────────────────────────────────────────────
    createWindow(W, H, "CDLib - feature demo");

    // Run for 10 seconds so every feature is visible, then exit.
    constexpr float DEMO_DURATION = 10.f;

    while (true)
    {
        // ── 2. getTime() – seconds since program start ─────────────────────
        float t = getTime();
        if (t > DEMO_DURATION)
            break;

        std::cerr << "t = " << t << std::endl;

        float phase = t / DEMO_DURATION; // 0 → 1 over the whole demo


        // ── 3. Clear with an animated background colour ────────────────────
        //    setFillColor drives clear() as well as filled shapes
        sf::Color bg(
            static_cast<uint8_t>(20 + 10 * std::sin(t)),
            static_cast<uint8_t>(20 + 10 * std::sin(t + 2.f)),
            static_cast<uint8_t>(40 + 10 * std::sin(t + 4.f))
        );
        setFillColor(bg);
        clear();


        // ── 4. drawLine ────────────────────────────────────────────────────
        //    Rotating "clock hand" from the centre of the window
        {
            float angle = 2.f * PI * phase;
            int cx = W / 2, cy = H / 2;
            int ex = cx + static_cast<int>(200.f * std::cos(angle));
            int ey = cy + static_cast<int>(200.f * std::sin(angle));

            setColor(sf::Color::White);
            setThinkness(2.f);
            drawLine(cx, cy, ex, ey);
        }


        // ── 5. drawCircle – bouncing ball ──────────────────────────────────
        {
            float bx = W * 0.5f + 250.f * std::sin(t * 1.3f);
            float by = H * 0.5f + 150.f * std::cos(t * 1.7f);

            setFillColor(hueColor(phase));
            setColor(sf::Color::White);
            setThinkness(2.f);
            drawCircle(static_cast<int>(bx), static_cast<int>(by), 35.f);
        }


        // ── 6. drawEllipse – pulsing oval in the top-left ─────────────────
        {
            float pulse = 1.f + 0.3f * std::sin(t * 3.f);
            int ex0 = 30, ey0 = 30;
            int ex1 = static_cast<int>(30 + 160 * pulse);
            int ey1 = static_cast<int>(30 + 70  * pulse);

            setFillColor(sf::Color(80, 40, 120, 200));
            setColor(sf::Color::Cyan);
            setThinkness(3.f);
            drawEllipse(ex0, ey0, ex1, ey1);
        }


        // ── 7. drawRect – animated rectangle in the top-right ─────────────
        {
            float wobble = std::abs(std::sin(t * 2.f));
            int rw = static_cast<int>(60 + 60 * wobble);
            int rh = static_cast<int>(40 + 40 * (1.f - wobble));
            int rx0 = W - 30 - rw, ry0 = 30;

            setFillColor(sf::Color(200, 100, 30, 180));
            setColor(sf::Color::Yellow);
            setThinkness(2.f);
            drawRect(rx0, ry0, rx0 + rw, ry0 + rh);
        }


        // ── 8. drawTriangle – spinning triangle near the bottom-left ──────
        {
            float angle = t * 1.5f;
            int cx = 120, cy = H - 120;
            float r = 80.f;

            auto vx = [&](int i){ return cx + static_cast<int>(r * std::cos(angle + i * 2.f * PI / 3.f)); };
            auto vy = [&](int i){ return cy + static_cast<int>(r * std::sin(angle + i * 2.f * PI / 3.f)); };

            setFillColor(sf::Color(30, 120, 200, 180));
            setColor(sf::Color::Green);
            setThinkness(2.f);
            drawTriangle(vx(0), vy(0), vx(1), vy(1), vx(2), vy(2));
        }


        // ── 9. drawPolygon – spinning pentagon near the bottom-right ──────
        {
            constexpr uint32_t N = 5;
            sf::Vector2f pts[N];
            float angle = -t;
            int cx = W - 130, cy = H - 120;
            float r = 80.f;

            for (uint32_t i = 0; i < N; ++i)
            {
                float a = angle + i * 2.f * PI / N;
                pts[i] = sf::Vector2f(cx + r * std::cos(a), cy + r * std::sin(a));
            }

            setFillColor(sf::Color(160, 40, 40, 180));
            setColor(sf::Color::Magenta);
            setThinkness(2.f);
            drawPolygon(pts, N);
        }


        // ── 10. setPixel / getPixel ────────────────────────────────────────
        //    Draw a small pixel-art dot-grid in the centre-bottom area;
        //    read one pixel back and tint it to show getPixel() works.
        {
            constexpr int GRID = 6;
            int startX = W / 2 - GRID * 4;
            int startY = H - 40;

            setColor(sf::Color::White); // setPixel uses the current "color"
            setFillColor(sf::Color::White);

            for (int i = 0; i < GRID; ++i)
            {
                // Alternate pixels on/off based on time
                bool on = ((i + static_cast<int>(t * 4)) % 2) == 0;
                if (on)
                    setPixel(startX + i * 8, startY);
            }

            // Read a pixel we just drew and use its colour to draw a marker
            sf::Color px = getPixel(startX, startY);
            setColor(px);
            setFillColor(sf::Color(px.r / 2, px.g / 2, px.b / 2, 200));
            setThinkness(1.f);
            drawCircle(startX, startY - 15, 6.f);
        }


        // ── 11. display + sleep ────────────────────────────────────────────
        //    display() flushes all queued draw commands to the screen.
        //    sleep() gives the OS time to process events and caps frame rate.
        display();
        sleep(1.f / FPS);
    }

    return 0;
}
