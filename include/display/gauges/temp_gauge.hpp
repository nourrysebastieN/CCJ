#pragma once

#include "gauge.hpp"

class TempGauge : public Gauge {
public:
    explicit TempGauge(SDL_Rect bounds);

    void set_value(float celsius) override;
    void render(SDL_Renderer* renderer) override;

private:
    float m_celsius{0.0f};

    static constexpr float TEMP_MIN      = 20.0f;
    static constexpr float TEMP_NORMAL   = 90.0f;
    static constexpr float TEMP_WARNING  = 105.0f;
    static constexpr float TEMP_MAX      = 120.0f;
};
