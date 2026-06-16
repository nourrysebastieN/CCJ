#include "display/layout.hpp"

namespace Layout {

std::vector<GaugeSlot> stacked(int width, int height) {
    constexpr int PAD = 20;
    constexpr int GAP = 12;
    const int bar_h = (height - PAD * 2 - GAP * 3) / 4;
    const int bar_w = width  - PAD * 2;
    auto y = [&](int row) { return PAD + row * (bar_h + GAP); };
    return {
        { GaugeType::RPM,          {PAD, y(0), bar_w, bar_h} },
        { GaugeType::SPEED,        {PAD, y(1), bar_w, bar_h} },
        { GaugeType::FUEL,         {PAD, y(2), bar_w, bar_h} },
        { GaugeType::COOLANT_TEMP, {PAD, y(3), bar_w, bar_h} },
    };
}

std::vector<GaugeSlot> grid_2x2(int width, int height) {
    constexpr int PAD = 20;
    constexpr int GAP = 12;
    const int w = (width  - PAD * 2 - GAP) / 2;
    const int h = (height - PAD * 2 - GAP) / 2;
    return {
        { GaugeType::RPM,          {PAD,             PAD,             w, h} },
        { GaugeType::SPEED,        {PAD + w + GAP,   PAD,             w, h} },
        { GaugeType::FUEL,         {PAD,             PAD + h + GAP,   w, h} },
        { GaugeType::COOLANT_TEMP, {PAD + w + GAP,   PAD + h + GAP,   w, h} },
    };
}

std::vector<GaugeSlot> analog_grid_2x2(int width, int height) {
    constexpr int PAD = 20;
    constexpr int GAP = 12;
    const int cell_w = (width  - PAD * 2 - GAP) / 2;
    const int cell_h = (height - PAD * 2 - GAP) / 2;
    const int dial   = cell_h;                  // square side = cell height
    const int dx     = (cell_w - dial) / 2;     // center horizontally in cell

    auto cx = [&](int col) { return PAD + col * (cell_w + GAP) + dx; };
    auto cy = [&](int row) { return PAD + row * (cell_h + GAP); };

    return {
        { GaugeType::RPM,          {cx(0), cy(0), dial, dial} },
        { GaugeType::SPEED,        {cx(1), cy(0), dial, dial} },
        { GaugeType::FUEL,         {cx(0), cy(1), dial, dial} },
        { GaugeType::COOLANT_TEMP, {cx(1), cy(1), dial, dial} },
    };
}

} // namespace Layout
