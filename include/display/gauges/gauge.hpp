#pragma once

#include <SDL2/SDL.h>
#include <string>

struct GaugeStyle {
    SDL_Color background{10, 10, 10, 255};
    SDL_Color foreground{220, 220, 220, 255};
    SDL_Color accent{200, 30, 30, 255}; // redline / warning color
};

// Base interface for all dashboard gauge widgets.
class Gauge {
public:
    virtual ~Gauge() = default;
    virtual void render(SDL_Renderer* renderer) = 0;
    virtual void set_value(float value) = 0;
    virtual void load_assets(SDL_Renderer* /*renderer*/) {}

protected:
    SDL_Rect m_bounds{};
    GaugeStyle m_style{};
};
