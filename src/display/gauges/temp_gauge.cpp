#include "display/gauges/temp_gauge.hpp"

#include <algorithm>

static void fill_rect(SDL_Renderer* r, SDL_Rect rect, SDL_Color c) {
    SDL_SetRenderDrawColor(r, c.r, c.g, c.b, c.a);
    SDL_RenderFillRect(r, &rect);
}

TempGauge::TempGauge(SDL_Rect bounds) {
    m_bounds = bounds;
}

void TempGauge::set_value(float celsius) {
    m_celsius = std::clamp(celsius, TEMP_MIN, TEMP_MAX);
}

void TempGauge::render(SDL_Renderer* renderer) {
    fill_rect(renderer, m_bounds, m_style.background);

    float ratio = (m_celsius - TEMP_MIN) / (TEMP_MAX - TEMP_MIN);
    SDL_Rect bar = m_bounds;
    bar.w = static_cast<int>(m_bounds.w * ratio);

    SDL_Color bar_color;
    if (m_celsius >= TEMP_WARNING)
        bar_color = m_style.accent;              // red — overheating
    else if (m_celsius >= TEMP_NORMAL)
        bar_color = {220, 220, 220, 255};        // white — normal operating
    else
        bar_color = {30, 120, 220, 255};         // blue — still warming up

    fill_rect(renderer, bar, bar_color);

    // Normal range marker at TEMP_NORMAL
    float normal_ratio = (TEMP_NORMAL - TEMP_MIN) / (TEMP_MAX - TEMP_MIN);
    int marker_x = m_bounds.x + static_cast<int>(m_bounds.w * normal_ratio);
    SDL_SetRenderDrawColor(renderer, 80, 80, 80, 255);
    SDL_RenderDrawLine(renderer, marker_x, m_bounds.y, marker_x, m_bounds.y + m_bounds.h);
}
