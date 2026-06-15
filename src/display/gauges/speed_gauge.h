#pragma once

#include "gauge.h"

class SpeedGauge : public Gauge {
public:
    explicit SpeedGauge(SDL_Rect bounds);

    void set_value(float kmh) override;
    void render(SDL_Renderer* renderer) override;

private:
    float m_kmh{0.0f};

    static constexpr float SPEED_MAX = 220.0f;
};
