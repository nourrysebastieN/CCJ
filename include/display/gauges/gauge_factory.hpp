#pragma once

#include "gauge.hpp"
#include <memory>

enum class GaugeType {
    RPM,
    SPEED,
    FUEL,
    COOLANT_TEMP,
};

class IGaugeFactory {
public:
    virtual ~IGaugeFactory() = default;
    virtual std::unique_ptr<Gauge> create(GaugeType type, SDL_Rect bounds) = 0;
    // Returns the path to a full-window background image, or "" for none.
    virtual std::string background_path() const { return ""; }
};
