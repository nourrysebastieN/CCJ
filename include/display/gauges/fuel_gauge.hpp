#pragma once

#include "gauge.hpp"

class FuelGauge : public Gauge {
public:
    explicit FuelGauge(SDL_Rect bounds);

    void set_value(float percent) override;
    void render(SDL_Renderer* renderer) override;

private:
    float m_percent{0.0f};

    static constexpr float LOW_FUEL_THRESHOLD = 15.0f;
};
