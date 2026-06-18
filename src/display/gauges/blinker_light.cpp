#include "display/gauges/blinker_light.hpp"

BlinkerLight::BlinkerLight(SDL_Rect bounds, Direction dir,
                           Uint32 blink_ms, SDL_Color on_color)
    : m_dir(dir), m_blink_ms(blink_ms), m_on_color(on_color)
{
    m_bounds = bounds;
}

void BlinkerLight::set_value(float value) {
    m_active = (value > 0.0f);
}

void BlinkerLight::draw_arrow(SDL_Renderer* r, SDL_Color color) const {
    SDL_SetRenderDrawColor(r, color.r, color.g, color.b, color.a);

    int x  = m_bounds.x;
    int y  = m_bounds.y;
    int w  = m_bounds.w;
    int h  = m_bounds.h;
    int cy = y + h / 2;
    int h2 = h / 2;

    for (int row = y; row < y + h; ++row) {
        int dist       = std::abs(row - cy);
        float portion  = (h2 > 0) ? static_cast<float>(dist) / h2 : 0.0f;

        if (m_dir == Direction::RIGHT) {
            // Tip at right; base at left
            int x_left  = x;
            int x_right = x + static_cast<int>(w * (1.0f - portion));
            if (x_right >= x_left)
                SDL_RenderDrawLine(r, x_left, row, x_right, row);
        } else {
            // Tip at left; base at right
            int x_left  = x + static_cast<int>(w * portion);
            int x_right = x + w;
            if (x_right >= x_left)
                SDL_RenderDrawLine(r, x_left, row, x_right, row);
        }
    }
}

void BlinkerLight::render(SDL_Renderer* renderer) {
    // Dim version: 1/5 brightness, preserving hue
    SDL_Color dim{
        static_cast<Uint8>(m_on_color.r / 5),
        static_cast<Uint8>(m_on_color.g / 5),
        static_cast<Uint8>(m_on_color.b / 5),
        255
    };

    if (!m_active) {
        draw_arrow(renderer, dim);
        return;
    }

    Uint32 ticks = SDL_GetTicks();
    bool   on    = (ticks / m_blink_ms) % 2 == 0;

    draw_arrow(renderer, on ? m_on_color : dim);
}
