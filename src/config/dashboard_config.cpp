#include "config/dashboard_config.hpp"

#include <nlohmann/json.hpp>

#include <fstream>
#include <iostream>
#include <unordered_map>

using json = nlohmann::json;

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------

static SDL_Color parse_color(const json& arr, SDL_Color fallback) {
    if (!arr.is_array() || arr.size() < 3) return fallback;
    return {
        static_cast<Uint8>(arr[0].get<int>()),
        static_cast<Uint8>(arr[1].get<int>()),
        static_cast<Uint8>(arr[2].get<int>()),
        arr.size() >= 4 ? static_cast<Uint8>(arr[3].get<int>()) : Uint8(255)
    };
}

static const std::unordered_map<std::string, GaugeType> GAUGE_TYPE_MAP = {
    { "RPM",           GaugeType::RPM           },
    { "SPEED",         GaugeType::SPEED         },
    { "FUEL",          GaugeType::FUEL          },
    { "COOLANT_TEMP",  GaugeType::COOLANT_TEMP  },
    { "BLINKER_LEFT",  GaugeType::BLINKER_LEFT  },
    { "BLINKER_RIGHT", GaugeType::BLINKER_RIGHT },
    { "WARNING_CEL",       GaugeType::WARNING_CEL       },
    { "WARNING_OIL",       GaugeType::WARNING_OIL       },
    { "WARNING_BAT",       GaugeType::WARNING_BAT       },
    { "WARNING_HAZARD",    GaugeType::WARNING_HAZARD    },
    { "WARNING_LOW_BEAM",  GaugeType::WARNING_LOW_BEAM  },
    { "WARNING_HIGH_BEAM", GaugeType::WARNING_HIGH_BEAM },
};

static GaugeType parse_gauge_type(const std::string& s) {
    auto it = GAUGE_TYPE_MAP.find(s);
    if (it == GAUGE_TYPE_MAP.end())
        throw std::runtime_error("Unknown gauge type: " + s);
    return it->second;
}

static GaugeTemplate parse_gauge_template(const json& j) {
    GaugeTemplate t;
    if (j.contains("min"))          t.min_value   = j["min"].get<float>();
    if (j.contains("max"))          t.max_value   = j["max"].get<float>();
    if (j.contains("start_angle"))  t.start_angle = j["start_angle"].get<float>();
    if (j.contains("sweep"))        t.sweep       = j["sweep"].get<float>();
    if (j.contains("major_ticks"))  t.major_ticks = j["major_ticks"].get<int>();
    if (j.contains("arc_color"))    t.arc_color    = parse_color(j["arc_color"],    t.arc_color);
    if (j.contains("tick_color"))   t.tick_color   = parse_color(j["tick_color"],   t.tick_color);
    if (j.contains("needle_color")) t.needle_color = parse_color(j["needle_color"], t.needle_color);
    if (j.contains("face_svg"))     t.face_svg     = j["face_svg"].get<std::string>();

    if (j.contains("zones")) {
        t.zones.clear();
        for (const auto& z : j["zones"]) {
            ColorZone cz;
            cz.from  = z.value("from", 0.0f);
            cz.to    = z.value("to",   0.0f);
            cz.color = parse_color(z.value("color", json::array()), {200, 30, 30, 255});
            t.zones.push_back(cz);
        }
    }
    return t;
}

static IndicatorConfig parse_indicator(const json& j) {
    IndicatorConfig cfg;
    if (j.contains("color"))      cfg.color      = parse_color(j["color"], cfg.color);
    if (j.contains("blink_ms"))   cfg.blink_ms   = j["blink_ms"].get<Uint32>();
    if (j.contains("symbol_svg")) cfg.symbol_svg = j["symbol_svg"].get<std::string>();
    return cfg;
}

// ---------------------------------------------------------------------------
// defaults()
// ---------------------------------------------------------------------------

DashboardConfig DashboardConfig::defaults() {
    DashboardConfig cfg;

    cfg.window  = { 1280, 560 };
    cfg.theme   = Theme::from("assets/themes", "default");

    // 2×2 grid (top 480px) — hand-computed for 1280×480
    // PAD=20, GAP=12 → cell w=614, h=214
    cfg.layout = {
        { GaugeType::RPM,          {  20,  20, 614, 214 } },
        { GaugeType::SPEED,        { 646,  20, 614, 214 } },
        { GaugeType::FUEL,         {  20, 246, 614, 214 } },
        { GaugeType::COOLANT_TEMP, { 646, 246, 614, 214 } },
        // Indicator strip at y=490
        { GaugeType::BLINKER_LEFT,  {   20, 490,  80, 60 } },
        { GaugeType::WARNING_OIL,   {  534, 490,  60, 60 } },
        { GaugeType::WARNING_CEL,   {  610, 490,  60, 60 } },
        { GaugeType::WARNING_BAT,   {  686, 490,  60, 60 } },
        { GaugeType::BLINKER_RIGHT, { 1180, 490,  80, 60 } },
    };

    cfg.gauge_templates[GaugeType::RPM] = {
        0.0f, 8000.0f, 225.0f, 270.0f,
        {180,180,180,255}, {220,220,220,255}, {30,200,30,255},
        { {6500.0f, 8000.0f, {200,30,30,255}} }, 8
    };
    cfg.gauge_templates[GaugeType::SPEED] = {
        0.0f, 220.0f, 225.0f, 270.0f,
        {180,180,180,255}, {220,220,220,255}, {30,200,30,255},
        {}, 11
    };
    cfg.gauge_templates[GaugeType::FUEL] = {
        0.0f, 100.0f, 225.0f, 270.0f,
        {180,180,180,255}, {220,220,220,255}, {30,200,30,255},
        { {0.0f, 15.0f, {220,180,0,255}} }, 4
    };
    cfg.gauge_templates[GaugeType::COOLANT_TEMP] = {
        20.0f, 120.0f, 225.0f, 270.0f,
        {180,180,180,255}, {220,220,220,255}, {30,200,30,255},
        { {20.0f,60.0f,{30,100,200,255}}, {105.0f,120.0f,{200,30,30,255}} }, 5
    };

    cfg.indicators[GaugeType::BLINKER_LEFT]       = { {255,165,  0,255}, 380 };
    cfg.indicators[GaugeType::BLINKER_RIGHT]      = { {255,165,  0,255}, 380 };
    cfg.indicators[GaugeType::WARNING_CEL]        = { {255,165,  0,255}, 0, "" };
    cfg.indicators[GaugeType::WARNING_OIL]        = { {220, 30, 30,255}, 0, "" };
    cfg.indicators[GaugeType::WARNING_BAT]        = { {220, 30, 30,255}, 0, "" };
    cfg.indicators[GaugeType::WARNING_HAZARD]     = { {255,165,  0,255}, 0, "" };
    cfg.indicators[GaugeType::WARNING_LOW_BEAM]   = { {  0,210,  0,255}, 0, "" };
    cfg.indicators[GaugeType::WARNING_HIGH_BEAM]  = { { 30,120,255,255}, 0, "" };

    return cfg;
}

// ---------------------------------------------------------------------------
// load()
// ---------------------------------------------------------------------------

DashboardConfig DashboardConfig::load(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        std::cerr << "[config] " << path << " not found — using defaults\n";
        return defaults();
    }

    json j;
    try {
        file >> j;
    } catch (const json::parse_error& e) {
        std::cerr << "[config] Parse error in " << path << ": " << e.what() << " — using defaults\n";
        return defaults();
    }

    DashboardConfig cfg = defaults();   // start from defaults, override with JSON values

    // Window
    if (j.contains("window")) {
        const auto& w = j["window"];
        cfg.window.width  = w.value("width",  cfg.window.width);
        cfg.window.height = w.value("height", cfg.window.height);
    }

    // Theme
    if (j.contains("theme")) {
        cfg.theme = Theme::from("assets/themes", j["theme"].get<std::string>());
    }

    // Layout
    if (j.contains("layout")) {
        cfg.layout.clear();
        for (const auto& slot : j["layout"]) {
            GaugeType type = parse_gauge_type(slot.value("type", "RPM"));
            SDL_Rect bounds {
                slot.value("x", 0),
                slot.value("y", 0),
                slot.value("w", 100),
                slot.value("h", 100),
            };
            bool enabled = slot.value("enabled", true);
            cfg.layout.push_back({ type, bounds, enabled });
        }
    }

    // Gauge templates and indicator configs
    if (j.contains("gauges")) {
        for (const auto& [key, val] : j["gauges"].items()) {
            GaugeType type;
            try { type = parse_gauge_type(key); }
            catch (...) {
                std::cerr << "[config] Unknown gauge key '" << key << "' — skipping\n";
                continue;
            }

            // Analog gauge template (has min/max or zones)
            bool is_template = val.contains("min") || val.contains("max")
                             || val.contains("zones") || val.contains("major_ticks")
                             || val.contains("arc_color");
            if (is_template) {
                GaugeTemplate base;
                auto it = cfg.gauge_templates.find(type);
                if (it != cfg.gauge_templates.end()) base = it->second;
                cfg.gauge_templates[type] = parse_gauge_template(val);
                // Re-apply base fields that weren't overridden
                auto& t = cfg.gauge_templates[type];
                if (!val.contains("min"))          t.min_value    = base.min_value;
                if (!val.contains("max"))          t.max_value    = base.max_value;
                if (!val.contains("start_angle"))  t.start_angle  = base.start_angle;
                if (!val.contains("sweep"))        t.sweep        = base.sweep;
                if (!val.contains("major_ticks"))  t.major_ticks  = base.major_ticks;
                if (!val.contains("arc_color"))    t.arc_color    = base.arc_color;
                if (!val.contains("tick_color"))   t.tick_color   = base.tick_color;
                if (!val.contains("needle_color")) t.needle_color = base.needle_color;
                if (!val.contains("zones"))        t.zones        = base.zones;
                if (!val.contains("face_svg"))     t.face_svg     = base.face_svg;
            }

            // Indicator config (color / blink_ms)
            bool is_indicator = val.contains("color") || val.contains("blink_ms");
            if (is_indicator) {
                IndicatorConfig base;
                auto it = cfg.indicators.find(type);
                if (it != cfg.indicators.end()) base = it->second;
                IndicatorConfig ind = parse_indicator(val);
                if (!val.contains("color"))      ind.color      = base.color;
                if (!val.contains("blink_ms"))   ind.blink_ms   = base.blink_ms;
                if (!val.contains("symbol_svg")) ind.symbol_svg = base.symbol_svg;
                cfg.indicators[type] = ind;
            }
        }
    }

    return cfg;
}
