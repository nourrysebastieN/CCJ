#include "display/gauges/fuel_gauge.hpp"

#include <algorithm>

static void fill_rect(SDL_Renderer* r, SDL_Rect rect, SDL_Color c) {
    SDL_SetRenderDrawColor(r, c.r, c.g, c.b, c.a);
    SDL_RenderFillRect(r, &rect);
}

FuelGauge::FuelGauge(SDL_Rect bounds) {
    m_bounds = bounds;
}

void FuelGauge::set_value(float percent) {
    m_percent = std::clamp(percent, 0.0f, 100.0f);
}

void FuelGauge::render(SDL_Renderer* renderer) {
    fill_rect(renderer, m_bounds, m_style.background);

    float ratio = m_percent / 100.0f;
    SDL_Rect bar = m_bounds;
    bar.w = static_cast<int>(m_bounds.w * ratio);

    // Yellow when low, green otherwise
    SDL_Color bar_color = (m_percent <= LOW_FUEL_THRESHOLD)
        ? SDL_Color{220, 180, 0, 255}
        : SDL_Color{30, 180, 60, 255};

    fill_rect(renderer, bar, bar_color);
}
