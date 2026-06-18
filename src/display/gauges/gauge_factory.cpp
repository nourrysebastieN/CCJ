#include "display/gauges/bar_gauge_factory.hpp"

#include "display/gauges/rpm_gauge.hpp"
#include "display/gauges/speed_gauge.hpp"
#include "display/gauges/fuel_gauge.hpp"
#include "display/gauges/temp_gauge.hpp"
#include "display/gauges/blinker_light.hpp"
#include "display/gauges/warning_light.hpp"

BarGaugeFactory::BarGaugeFactory(const DashboardConfig& config)
    : m_theme(config.theme)
    , m_indicators(config.indicators) {}

std::string BarGaugeFactory::background_path() const {
    if (m_theme.dir.empty()) return "";
    return m_theme.dir + "/dashboard_bg.png";
}

std::unique_ptr<Gauge> BarGaugeFactory::create(GaugeType type, SDL_Rect bounds) {
    switch (type) {
        case GaugeType::RPM:          return std::make_unique<RpmGauge>(bounds);
        case GaugeType::SPEED:        return std::make_unique<SpeedGauge>(bounds);
        case GaugeType::FUEL:         return std::make_unique<FuelGauge>(bounds);
        case GaugeType::COOLANT_TEMP: return std::make_unique<TempGauge>(bounds);

        case GaugeType::BLINKER_LEFT:
        case GaugeType::BLINKER_RIGHT: {
            auto      dir      = (type == GaugeType::BLINKER_LEFT)
                                 ? BlinkerLight::Direction::LEFT
                                 : BlinkerLight::Direction::RIGHT;
            Uint32    blink    = 380;
            SDL_Color on_color = {255, 165, 0, 255};
            auto it = m_indicators.find(type);
            if (it != m_indicators.end()) {
                blink    = it->second.blink_ms;
                on_color = it->second.color;
            }
            return std::make_unique<BlinkerLight>(bounds, dir, blink, on_color);
        }

        case GaugeType::WARNING_CEL:
        case GaugeType::WARNING_OIL:
        case GaugeType::WARNING_BAT:
        case GaugeType::WARNING_HAZARD:
        case GaugeType::WARNING_LOW_BEAM:
        case GaugeType::WARNING_HIGH_BEAM: {
            SDL_Color   color{255, 165, 0, 255};
            std::string svg;
            auto it = m_indicators.find(type);
            if (it != m_indicators.end()) {
                color = it->second.color;
                svg   = it->second.symbol_svg;
            }
            return std::make_unique<WarningLight>(bounds, color, std::move(svg));
        }
    }
    return nullptr;
}
