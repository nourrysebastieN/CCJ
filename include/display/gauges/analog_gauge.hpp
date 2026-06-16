#pragma once

#include "gauge.hpp"
#include <string>

class AnalogGauge : public Gauge {
public:
    AnalogGauge(SDL_Rect bounds, float min, float max,
                std::string bg_path, std::string needle_path);
    ~AnalogGauge() override;

    void set_value(float value) override;
    void load_assets(SDL_Renderer* renderer) override;
    void render(SDL_Renderer* renderer) override;

private:
    float m_min;
    float m_max;
    float m_value{0.0f};

    std::string  m_bg_path;
    std::string  m_needle_path;
    SDL_Texture* m_bg_tex{nullptr};
    SDL_Texture* m_needle_tex{nullptr};

    // Needle sweeps 270° clockwise: 0 at 225° (bottom-left), max at 135° (bottom-right).
    static constexpr float ANGLE_MIN_DEG   = 225.0f;
    static constexpr float ANGLE_SWEEP_DEG = 270.0f;
};
