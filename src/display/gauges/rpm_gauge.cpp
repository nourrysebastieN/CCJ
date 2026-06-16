#include "display/gauges/rpm_gauge.hpp"

#include <algorithm>
#include <cmath>
#include <string>

static void draw_filled_rect(SDL_Renderer* r, SDL_Rect rect, SDL_Color c) {
    SDL_SetRenderDrawColor(r, c.r, c.g, c.b, c.a);
    SDL_RenderFillRect(r, &rect);
}

RpmGauge::RpmGauge(SDL_Rect bounds) {
    m_bounds = bounds;
}

void RpmGauge::set_value(float rpm) {
    m_rpm = std::clamp(rpm, 0.0f, RPM_MAX);
}

void RpmGauge::render(SDL_Renderer* renderer) {
    // Background
    draw_filled_rect(renderer, m_bounds, m_style.background);

    // Bar fill
    float ratio = m_rpm / RPM_MAX;
    bool  in_redline = m_rpm >= RPM_REDLINE;

    SDL_Rect bar = m_bounds;
    bar.w = static_cast<int>(m_bounds.w * ratio);
    draw_filled_rect(renderer, bar, in_redline ? m_style.accent : m_style.foreground);

    // Redline marker
    int redline_x = m_bounds.x + static_cast<int>(m_bounds.w * (RPM_REDLINE / RPM_MAX));
    SDL_SetRenderDrawColor(renderer, m_style.accent.r, m_style.accent.g, m_style.accent.b, 255);
    SDL_RenderDrawLine(renderer, redline_x, m_bounds.y, redline_x, m_bounds.y + m_bounds.h);
}
