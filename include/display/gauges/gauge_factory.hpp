#pragma once

#include "gauge.hpp"
#include <memory>

enum class GaugeType {
    RPM,
    SPEED,
    FUEL,
    COOLANT_TEMP,
    // Indicators
    BLINKER_LEFT,
    BLINKER_RIGHT,
    WARNING_CEL,        // check-engine light
    WARNING_OIL,
    WARNING_BAT,
    WARNING_HAZARD,     // hazard triangle
    WARNING_LOW_BEAM,   // low-beam headlights on
    WARNING_HIGH_BEAM,  // high-beam headlights on
};

class IGaugeFactory {
public:
    virtual ~IGaugeFactory() = default;
    virtual std::unique_ptr<Gauge> create(GaugeType type, SDL_Rect bounds) = 0;
    // Returns the path to a full-window background image, or "" for none.
    virtual std::string background_path() const { return ""; }
};
