#include "display/gauges/analog_gauge_factory.hpp"
#include "display/gauges/analog_gauge.hpp"
#include "display/gauges/blinker_light.hpp"
#include "display/gauges/warning_light.hpp"

AnalogGaugeFactory::AnalogGaugeFactory(const DashboardConfig& config)
    : m_theme(config.theme)
    , m_templates(config.gauge_templates)
    , m_indicators(config.indicators) {}

std::string AnalogGaugeFactory::background_path() const {
    return m_theme.dir.empty() ? "" : m_theme.dir + "/dashboard_bg.png";
}

std::unique_ptr<Gauge> AnalogGaugeFactory::create(GaugeType type, SDL_Rect bounds) {
    // Blinker lights
    if (type == GaugeType::BLINKER_LEFT || type == GaugeType::BLINKER_RIGHT) {
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

    // Warning lights
    if (type == GaugeType::WARNING_CEL  || type == GaugeType::WARNING_OIL
     || type == GaugeType::WARNING_BAT  || type == GaugeType::WARNING_HAZARD
     || type == GaugeType::WARNING_LOW_BEAM || type == GaugeType::WARNING_HIGH_BEAM) {
        SDL_Color   color{255, 165, 0, 255};
        std::string svg;
        auto it = m_indicators.find(type);
        if (it != m_indicators.end()) {
            color = it->second.color;
            svg   = it->second.symbol_svg;
        }
        return std::make_unique<WarningLight>(bounds, color, std::move(svg));
    }

    // Analog gauges — look up template from config
    GaugeTemplate tmpl;
    auto it = m_templates.find(type);
    if (it != m_templates.end()) {
        tmpl = it->second;
    } else {
        // Sensible hardcoded fallbacks for any type not in the config.
        switch (type) {
            case GaugeType::RPM:
                tmpl.min_value = 0; tmpl.max_value = 8000; tmpl.major_ticks = 8;
                tmpl.zones = { {6500.0f, 8000.0f, {200, 30, 30, 255}} };
                break;
            case GaugeType::SPEED:
                tmpl.min_value = 0; tmpl.max_value = 220; tmpl.major_ticks = 11;
                break;
            case GaugeType::FUEL:
                tmpl.min_value = 0; tmpl.max_value = 100; tmpl.major_ticks = 4;
                tmpl.zones = { {0.0f, 15.0f, {220, 180, 0, 255}} };
                break;
            case GaugeType::COOLANT_TEMP:
                tmpl.min_value = 20; tmpl.max_value = 120; tmpl.major_ticks = 5;
                tmpl.zones = {
                    {20.0f,  60.0f,  {30,  100, 200, 255}},
                    {105.0f, 120.0f, {200, 30,  30,  255}},
                };
                break;
            default: break;
        }
    }

    return std::make_unique<AnalogGauge>(bounds, std::move(tmpl));
}
