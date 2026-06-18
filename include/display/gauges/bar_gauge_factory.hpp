#pragma once

#include "gauge_factory.hpp"
#include "display/theme.hpp"
#include "config/dashboard_config.hpp"

class BarGaugeFactory : public IGaugeFactory {
public:
    explicit BarGaugeFactory(const DashboardConfig& config);

    std::unique_ptr<Gauge> create(GaugeType type, SDL_Rect bounds) override;
    std::string background_path() const override;

private:
    Theme                                m_theme;
    std::map<GaugeType, IndicatorConfig> m_indicators;
};
