#pragma once

#include "gauges/gauge_factory.hpp"
#include <SDL2/SDL.h>
#include <vector>

struct GaugeSlot {
    GaugeType type;
    SDL_Rect  bounds;
    bool      enabled{true};
};

namespace Layout {
    // Four equal horizontal bars stacked vertically — suits bar-fill gauges.
    std::vector<GaugeSlot> stacked(int width, int height);

    // 2×2 grid of rectangular cells — general purpose.
    std::vector<GaugeSlot> grid_2x2(int width, int height);

    // 2×2 grid where each gauge bounds is a square (side = cell height),
    // centered horizontally in its cell — prevents circular dials from
    // being stretched into ellipses.
    std::vector<GaugeSlot> analog_grid_2x2(int width, int height);
}
