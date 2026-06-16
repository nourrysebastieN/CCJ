#pragma once

#include "gauge_factory.hpp"
#include <string>

class AnalogGaugeFactory : public IGaugeFactory {
public:
    // assets_dir must contain: rpm_bg.png, speed_bg.png, fuel_bg.png,
    //                           temp_bg.png, and needle.png.
    // If any file is missing, the affected gauge falls back to stub rendering.
    explicit AnalogGaugeFactory(std::string assets_dir = "assets");

    std::unique_ptr<Gauge> create(GaugeType type, SDL_Rect bounds) override;
    std::string background_path() const override;

private:
    std::string m_assets_dir;
    std::string path(const std::string& filename) const;
};
