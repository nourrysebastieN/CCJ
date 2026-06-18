#pragma once

#include "gauge.hpp"
#include <string>

class WarningLight : public Gauge {
public:
    WarningLight(SDL_Rect bounds, SDL_Color color, std::string symbol_svg = "");
    ~WarningLight();

    void set_value(float value) override;   // > 0 = active
    void render(SDL_Renderer* renderer) override;
    void load_assets(SDL_Renderer* renderer) override;

private:
    SDL_Color    m_color;
    std::string  m_symbol_svg;
    bool         m_active{false};
    SDL_Texture* m_sym_tex{nullptr};

    void fill_circle(SDL_Renderer* r, int cx, int cy, int radius, SDL_Color color) const;
};
