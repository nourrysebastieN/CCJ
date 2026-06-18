#pragma once

#include "display/gauges/gauge_factory.hpp"
#include "display/gauges/gauge_template.hpp"
#include "display/layout.hpp"
#include "display/theme.hpp"

#include <SDL2/SDL.h>
#include <map>
#include <string>
#include <vector>

struct WindowConfig {
    int width{1280};
    int height{560};
};

// Per-indicator (blinker / warning light) configuration.
struct IndicatorConfig {
    SDL_Color   color{255, 165, 0, 255};
    Uint32      blink_ms{380};
    std::string symbol_svg;   // path to white-on-transparent SVG icon (optional)
};

struct DashboardConfig {
    WindowConfig                          window;
    Theme                                 theme;
    std::vector<GaugeSlot>                layout;
    std::map<GaugeType, GaugeTemplate>    gauge_templates;
    std::map<GaugeType, IndicatorConfig>  indicators;

    // Load from JSON file; falls back to defaults() if the file is missing or malformed.
    static DashboardConfig load(const std::string& path);

    // Hardcoded defaults — identical to what the code had before JSON support.
    static DashboardConfig defaults();
};
