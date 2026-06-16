#include "display/gauges/analog_gauge.hpp"

#ifdef CCJ_HAS_SDL2_IMAGE
#include <SDL2/SDL_image.h>
#endif

AnalogGauge::AnalogGauge(SDL_Rect bounds, float min, float max,
                         std::string bg_path, std::string needle_path)
    : m_min(min), m_max(max)
    , m_bg_path(std::move(bg_path))
    , m_needle_path(std::move(needle_path))
{
    m_bounds = bounds;
}

AnalogGauge::~AnalogGauge() {
    if (m_bg_tex)     SDL_DestroyTexture(m_bg_tex);
    if (m_needle_tex) SDL_DestroyTexture(m_needle_tex);
}

void AnalogGauge::set_value(float value) {
    m_value = value;
}

void AnalogGauge::load_assets(SDL_Renderer* renderer) {
#ifdef CCJ_HAS_SDL2_IMAGE
    auto load = [&](const std::string& path) -> SDL_Texture* {
        SDL_Surface* surf = IMG_Load(path.c_str());
        if (!surf) return nullptr;
        SDL_Texture* tex = SDL_CreateTextureFromSurface(renderer, surf);
        SDL_FreeSurface(surf);
        return tex;
    };
    m_bg_tex     = load(m_bg_path);
    m_needle_tex = load(m_needle_path);
#else
    (void)renderer;
#endif
}

void AnalogGauge::render(SDL_Renderer* renderer) {
    if (!m_bg_tex || !m_needle_tex)
        return;

    SDL_RenderCopy(renderer, m_bg_tex, nullptr, &m_bounds);

    float ratio = (m_value - m_min) / (m_max - m_min);
    double angle = static_cast<double>(ANGLE_MIN_DEG + ratio * ANGLE_SWEEP_DEG);

    SDL_Point pivot { m_bounds.w / 2, m_bounds.h / 2 };
    SDL_RenderCopyEx(renderer, m_needle_tex, nullptr, &m_bounds, angle, &pivot, SDL_FLIP_NONE);
}
