#pragma once

#include "gauge.hpp"

class RpmGauge : public Gauge {
public:
    RpmGauge(SDL_Rect bounds);

    void set_value(float rpm) override;
    void render(SDL_Renderer* renderer) override;

private:
    float m_rpm{0.0f};

    static constexpr float RPM_MAX     = 8000.0f;
    static constexpr float RPM_REDLINE = 6500.0f;
};
