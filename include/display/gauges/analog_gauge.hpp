#pragma once

#include "gauge.hpp"
#include "gauge_template.hpp"

class AnalogGauge : public Gauge {
public:
    AnalogGauge(SDL_Rect bounds, GaugeTemplate tmpl);
    ~AnalogGauge();

    void load_assets(SDL_Renderer* renderer) override;
    void set_value(float value) override;
    void render(SDL_Renderer* renderer) override;

private:
    GaugeTemplate m_tmpl;
    float         m_value{0.0f};
    SDL_Texture*  m_arc_tex{nullptr};  // cached arc + zones + ticks

    float angle_for(float value) const;

    // Draw helpers — work in whichever coordinate system the renderer targets.
    void draw_arc   (SDL_Renderer* r, float cx, float cy,
                     float inner_r, float outer_r,
                     float from_val, float to_val, SDL_Color color) const;
    void draw_ticks (SDL_Renderer* r, float cx, float cy,
                     float inner_r, float outer_r) const;
    void draw_needle(SDL_Renderer* r, float cx, float cy, float length) const;
};
