#include "speed_gauge.h"

#include <algorithm>
#include <cmath>

static void fill_rect(SDL_Renderer* r, SDL_Rect rect, SDL_Color c) {
    SDL_SetRenderDrawColor(r, c.r, c.g, c.b, c.a);
    SDL_RenderFillRect(r, &rect);
}

SpeedGauge::SpeedGauge(SDL_Rect bounds) {
    m_bounds = bounds;
}

void SpeedGauge::set_value(float kmh) {
    m_kmh = std::clamp(kmh, 0.0f, SPEED_MAX);
}

void SpeedGauge::render(SDL_Renderer* renderer) {
    fill_rect(renderer, m_bounds, m_style.background);

    float ratio = m_kmh / SPEED_MAX;
    SDL_Rect bar = m_bounds;
    bar.w = static_cast<int>(m_bounds.w * ratio);

    // Shift from green (low) to white (high) as speed increases
    SDL_Color bar_color{
        static_cast<Uint8>(std::min(255.0f, 100.0f + 155.0f * ratio)),
        static_cast<Uint8>(220.0f - 80.0f * ratio),
        static_cast<Uint8>(220.0f - 80.0f * ratio),
        255
    };
    fill_rect(renderer, bar, bar_color);
}
