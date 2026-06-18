#include "display/gauges/warning_light.hpp"

#ifdef CCJ_HAS_SDL2_IMAGE
#include <SDL2/SDL_image.h>
#endif

#include <cmath>
#include <algorithm>
#include <iostream>

WarningLight::WarningLight(SDL_Rect bounds, SDL_Color color, std::string symbol_svg)
    : m_color(color), m_symbol_svg(std::move(symbol_svg))
{
    m_bounds = bounds;
}

WarningLight::~WarningLight() {
    if (m_sym_tex) SDL_DestroyTexture(m_sym_tex);
}

void WarningLight::load_assets(SDL_Renderer* renderer) {
#ifdef CCJ_HAS_SDL2_IMAGE
    if (m_symbol_svg.empty()) return;

    SDL_Surface* raw = IMG_Load(m_symbol_svg.c_str());
    if (!raw) {
        std::cerr << "[warning] SVG load failed (" << m_symbol_svg
                  << "): " << IMG_GetError() << "\n";
        return;
    }

    SDL_Surface* scaled = SDL_CreateRGBSurfaceWithFormat(
        0, m_bounds.w, m_bounds.h, 32, SDL_PIXELFORMAT_RGBA32);
    if (scaled) {
        SDL_FillRect(scaled, nullptr, SDL_MapRGBA(scaled->format, 0, 0, 0, 0));
        SDL_SetSurfaceBlendMode(raw, SDL_BLENDMODE_NONE);
        SDL_BlitScaled(raw, nullptr, scaled, nullptr);
        m_sym_tex = SDL_CreateTextureFromSurface(renderer, scaled);
        if (m_sym_tex)
            SDL_SetTextureBlendMode(m_sym_tex, SDL_BLENDMODE_BLEND);
        SDL_FreeSurface(scaled);
    }
    SDL_FreeSurface(raw);
#endif
}

void WarningLight::set_value(float value) {
    m_active = (value > 0.0f);
}

void WarningLight::fill_circle(SDL_Renderer* r, int cx, int cy, int radius, SDL_Color color) const {
    SDL_SetRenderDrawColor(r, color.r, color.g, color.b, color.a);
    for (int dy = -radius; dy <= radius; ++dy) {
        int dx = static_cast<int>(std::sqrt(static_cast<float>(radius * radius - dy * dy)));
        SDL_RenderDrawLine(r, cx - dx, cy + dy, cx + dx, cy + dy);
    }
}

void WarningLight::render(SDL_Renderer* renderer) {
    if (m_sym_tex) {
        // White SVG texture × color mod = configured warning color.
        // Alpha is full when active, very dim when inactive.
        Uint8 alpha = m_active ? 255 : 30;
        SDL_SetTextureColorMod(m_sym_tex, m_color.r, m_color.g, m_color.b);
        SDL_SetTextureAlphaMod(m_sym_tex, alpha);
        SDL_RenderCopy(renderer, m_sym_tex, nullptr, &m_bounds);
        return;
    }

    // Fallback: solid circle (used when no SVG is configured)
    int cx     = m_bounds.x + m_bounds.w / 2;
    int cy     = m_bounds.y + m_bounds.h / 2;
    int radius = std::min(m_bounds.w, m_bounds.h) / 2 - 2;

    if (m_active) {
        fill_circle(renderer, cx, cy, radius, m_color);
    } else {
        SDL_Color dim{
            static_cast<Uint8>(m_color.r / 6),
            static_cast<Uint8>(m_color.g / 6),
            static_cast<Uint8>(m_color.b / 6),
            255
        };
        SDL_SetRenderDrawColor(renderer, dim.r, dim.g, dim.b, dim.a);
        for (int angle = 0; angle < 360; ++angle) {
            float rad = angle * 3.14159265f / 180.0f;
            int px = cx + static_cast<int>(radius * std::cos(rad));
            int py = cy + static_cast<int>(radius * std::sin(rad));
            SDL_RenderDrawPoint(renderer, px, py);
        }
    }
}
