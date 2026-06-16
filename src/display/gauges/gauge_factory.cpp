#include "display/gauges/bar_gauge_factory.hpp"

#include "display/gauges/rpm_gauge.hpp"
#include "display/gauges/speed_gauge.hpp"
#include "display/gauges/fuel_gauge.hpp"
#include "display/gauges/temp_gauge.hpp"

BarGaugeFactory::BarGaugeFactory(std::string assets_dir)
    : m_assets_dir(std::move(assets_dir)) {}

std::string BarGaugeFactory::background_path() const {
    if (m_assets_dir.empty()) return "";
    return m_assets_dir + "/dashboard_bg.png";
}

std::unique_ptr<Gauge> BarGaugeFactory::create(GaugeType type, SDL_Rect bounds) {
    switch (type) {
        case GaugeType::RPM:          return std::make_unique<RpmGauge>(bounds);
        case GaugeType::SPEED:        return std::make_unique<SpeedGauge>(bounds);
        case GaugeType::FUEL:         return std::make_unique<FuelGauge>(bounds);
        case GaugeType::COOLANT_TEMP: return std::make_unique<TempGauge>(bounds);
    }
    return nullptr;
}
