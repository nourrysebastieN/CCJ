#pragma once

#include "gauge.hpp"

class BlinkerLight : public Gauge {
public:
    enum class Direction { LEFT, RIGHT };

    BlinkerLight(SDL_Rect bounds, Direction dir,
                 Uint32 blink_ms = 380,
                 SDL_Color on_color = {255, 165, 0, 255});

    void set_value(float value) override;   // > 0 = active
    void render(SDL_Renderer* renderer) override;

private:
    Direction m_dir;
    bool      m_active{false};
    Uint32    m_blink_ms;
    SDL_Color m_on_color;

    void draw_arrow(SDL_Renderer* r, SDL_Color color) const;
};
