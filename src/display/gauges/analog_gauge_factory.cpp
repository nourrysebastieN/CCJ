#include "display/gauges/analog_gauge_factory.hpp"
#include "display/gauges/analog_gauge.hpp"

AnalogGaugeFactory::AnalogGaugeFactory(std::string assets_dir)
    : m_assets_dir(std::move(assets_dir)) {}

std::string AnalogGaugeFactory::path(const std::string& filename) const {
    return m_assets_dir + "/" + filename;
}

std::string AnalogGaugeFactory::background_path() const {
    return path("dashboard_bg.png");
}

std::unique_ptr<Gauge> AnalogGaugeFactory::create(GaugeType type, SDL_Rect bounds) {
    const std::string needle = path("needle.png");

    switch (type) {
        case GaugeType::RPM:
            return std::make_unique<AnalogGauge>(bounds,   0.0f, 8000.0f, path("rpm_bg.png"),   needle);
        case GaugeType::SPEED:
            return std::make_unique<AnalogGauge>(bounds,   0.0f,  220.0f, path("speed_bg.png"), needle);
        case GaugeType::FUEL:
            return std::make_unique<AnalogGauge>(bounds,   0.0f,  100.0f, path("fuel_bg.png"),  needle);
        case GaugeType::COOLANT_TEMP:
            return std::make_unique<AnalogGauge>(bounds,  20.0f,  120.0f, path("temp_bg.png"),  needle);
    }
    return nullptr;
}
