#include "display/gauges/analog_gauge.hpp"

#ifdef CCJ_HAS_SDL2_IMAGE
#include <SDL2/SDL_image.h>
#endif

#include <cmath>
#include <algorithm>
#include <iostream>

static constexpr float PI = 3.14159265f;

AnalogGauge::AnalogGauge(SDL_Rect bounds, GaugeTemplate tmpl)
    : m_tmpl(std::move(tmpl))
{
    m_bounds = bounds;
}

AnalogGauge::~AnalogGauge() {
    if (m_arc_tex) SDL_DestroyTexture(m_arc_tex);
}

// ---------------------------------------------------------------------------
// Texture cache — arc + zones + ticks drawn once, reused every frame
// ---------------------------------------------------------------------------

void AnalogGauge::load_assets(SDL_Renderer* renderer) {
    if (m_arc_tex) { SDL_DestroyTexture(m_arc_tex); m_arc_tex = nullptr; }

    // --- SVG face (optional) -------------------------------------------
    if (!m_tmpl.face_svg.empty()) {
#ifdef CCJ_HAS_SDL2_IMAGE
        SDL_Surface* raw = IMG_Load(m_tmpl.face_svg.c_str());
        if (raw) {
            // Scale to the gauge bounds so any SVG canvas size works.
            SDL_Surface* scaled = SDL_CreateRGBSurfaceWithFormat(
                0, m_bounds.w, m_bounds.h, 32, SDL_PIXELFORMAT_RGBA32);
            if (scaled) {
                // Clear to fully transparent before blitting so areas with
                // no SVG content stay transparent (not garbage/white).
                SDL_FillRect(scaled, nullptr,
                             SDL_MapRGBA(scaled->format, 0, 0, 0, 0));
                // BLENDMODE_NONE copies RGBA as-is (no compositing),
                // preserving the SVG's own alpha values.
                SDL_SetSurfaceBlendMode(raw, SDL_BLENDMODE_NONE);
                SDL_BlitScaled(raw, nullptr, scaled, nullptr);
                m_arc_tex = SDL_CreateTextureFromSurface(renderer, scaled);
                if (m_arc_tex)
                    SDL_SetTextureBlendMode(m_arc_tex, SDL_BLENDMODE_BLEND);
                SDL_FreeSurface(scaled);
            }
            SDL_FreeSurface(raw);
        } else {
            std::cerr << "[gauge] SVG load failed (" << m_tmpl.face_svg
                      << "): " << IMG_GetError() << " — falling back to programmatic drawing\n";
        }
#else
        std::cerr << "[gauge] face_svg set but SDL2_image is not available"
                     " — falling back to programmatic drawing\n";
#endif
        if (m_arc_tex) return;   // SVG loaded successfully, skip programmatic path
    }

    // --- Programmatic drawing (default / fallback) ----------------------
    m_arc_tex = SDL_CreateTexture(renderer,
                                  SDL_PIXELFORMAT_RGBA8888,
                                  SDL_TEXTUREACCESS_TARGET,
                                  m_bounds.w, m_bounds.h);
    if (!m_arc_tex) return;
    SDL_SetTextureBlendMode(m_arc_tex, SDL_BLENDMODE_BLEND);

    SDL_Texture* prev = SDL_GetRenderTarget(renderer);
    SDL_SetRenderTarget(renderer, m_arc_tex);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
    SDL_RenderClear(renderer);

    float cx = m_bounds.w / 2.0f;
    float cy = m_bounds.h / 2.0f;
    float r  = std::min(m_bounds.w, m_bounds.h) / 2.0f - 4.0f;

    draw_arc(renderer, cx, cy, r * 0.78f, r * 0.90f,
             m_tmpl.min_value, m_tmpl.max_value, m_tmpl.arc_color);

    for (const auto& zone : m_tmpl.zones)
        draw_arc(renderer, cx, cy, r * 0.78f, r * 0.90f,
                 zone.from, zone.to, zone.color);

    draw_ticks(renderer, cx, cy, r * 0.70f, r * 0.90f);

    SDL_SetRenderTarget(renderer, prev);
}

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------

float AnalogGauge::angle_for(float value) const {
    float ratio = (value - m_tmpl.min_value) / (m_tmpl.max_value - m_tmpl.min_value);
    return m_tmpl.start_angle + ratio * m_tmpl.sweep;
}

// Adaptive-step filled arc band.
// step = asin(1 / outer_r) so adjacent spokes are always ≤ 1 px apart at
// the outer edge — gap-free at any radius without over-rendering.
void AnalogGauge::draw_arc(SDL_Renderer* r, float cx, float cy,
                           float inner_r, float outer_r,
                           float from_val, float to_val, SDL_Color color) const {
    float a_start = angle_for(from_val);
    float a_end   = angle_for(to_val);

    float step_deg = (outer_r > 1.0f)
                     ? std::max(std::asin(1.0f / outer_r) * (180.0f / PI), 0.05f)
                     : 0.5f;

    SDL_SetRenderDrawColor(r, color.r, color.g, color.b, color.a);

    for (float a = a_start; a <= a_end + step_deg * 0.5f; a += step_deg) {
        float rad = std::min(a, a_end) * PI / 180.0f;
        float s = std::sin(rad), c = std::cos(rad);
        SDL_RenderDrawLine(r,
            static_cast<int>(cx + inner_r * s), static_cast<int>(cy - inner_r * c),
            static_cast<int>(cx + outer_r * s), static_cast<int>(cy - outer_r * c));
    }
}

void AnalogGauge::draw_ticks(SDL_Renderer* r, float cx, float cy,
                             float inner_r, float outer_r) const {
    SDL_SetRenderDrawColor(r,
        m_tmpl.tick_color.r, m_tmpl.tick_color.g,
        m_tmpl.tick_color.b, m_tmpl.tick_color.a);

    for (int i = 0; i <= m_tmpl.major_ticks; ++i) {
        float val = m_tmpl.min_value
                    + static_cast<float>(i) / m_tmpl.major_ticks
                    * (m_tmpl.max_value - m_tmpl.min_value);
        float rad = angle_for(val) * PI / 180.0f;
        float s = std::sin(rad), c = std::cos(rad);
        SDL_RenderDrawLine(r,
            static_cast<int>(cx + inner_r * s), static_cast<int>(cy - inner_r * c),
            static_cast<int>(cx + outer_r * s), static_cast<int>(cy - outer_r * c));
    }
}

void AnalogGauge::draw_needle(SDL_Renderer* r, float cx, float cy, float length) const {
    float rad = angle_for(m_value) * PI / 180.0f;
    float s = std::sin(rad), c = std::cos(rad);

    SDL_SetRenderDrawColor(r,
        m_tmpl.needle_color.r, m_tmpl.needle_color.g,
        m_tmpl.needle_color.b, m_tmpl.needle_color.a);

    for (int offset = -1; offset <= 1; ++offset) {
        float ox =  offset * c;
        float oy = -offset * s;
        SDL_RenderDrawLine(r,
            static_cast<int>(cx + ox), static_cast<int>(cy + oy),
            static_cast<int>(cx + length * s + ox),
            static_cast<int>(cy - length * c + oy));
    }
}

// ---------------------------------------------------------------------------
// Per-frame render: texture blit + needle only
// ---------------------------------------------------------------------------

void AnalogGauge::set_value(float value) {
    m_value = value;
}

void AnalogGauge::render(SDL_Renderer* renderer) {
    if (m_arc_tex)
        SDL_RenderCopy(renderer, m_arc_tex, nullptr, &m_bounds);

    float cx = m_bounds.x + m_bounds.w / 2.0f;
    float cy = m_bounds.y + m_bounds.h / 2.0f;
    float r  = std::min(m_bounds.w, m_bounds.h) / 2.0f - 4.0f;

    draw_needle(renderer, cx, cy, r * 0.65f);
}
