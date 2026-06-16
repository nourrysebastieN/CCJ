#pragma once

#include "gauge_factory.hpp"
#include <string>

class BarGaugeFactory : public IGaugeFactory {
public:
    explicit BarGaugeFactory(std::string assets_dir = "");

    std::unique_ptr<Gauge> create(GaugeType type, SDL_Rect bounds) override;
    std::string background_path() const override;

private:
    std::string m_assets_dir;
};
