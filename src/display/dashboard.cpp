#include "dashboard.h"

#include "gauges/fuel_gauge.h"
#include "gauges/rpm_gauge.h"
#include "gauges/speed_gauge.h"
#include "gauges/temp_gauge.h"

Dashboard::Dashboard(int width, int height)
    : m_width(width), m_height(height) {}

Dashboard::~Dashboard() {
    if (m_renderer) SDL_DestroyRenderer(m_renderer);
    if (m_window)   SDL_DestroyWindow(m_window);
    SDL_Quit();
}

bool Dashboard::init() {
    if (SDL_Init(SDL_INIT_VIDEO) != 0)
        return false;

    m_window = SDL_CreateWindow(
        "CCJ Dashboard",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        m_width, m_height,
        SDL_WINDOW_SHOWN
    );
    if (!m_window) return false;

    m_renderer = SDL_CreateRenderer(m_window, -1, SDL_RENDERER_ACCELERATED);
    if (!m_renderer) return false;

    build_layout();
    return true;
}

// Layout — 800x480, four horizontal bars stacked with padding
//
//  ┌─────────────────────────────┐
//  │  RPM                        │
//  ├─────────────────────────────┤
//  │  Speed                      │
//  ├─────────────────────────────┤
//  │  Fuel                       │
//  ├─────────────────────────────┤
//  │  Coolant temp               │
//  └─────────────────────────────┘
void Dashboard::build_layout() {
    constexpr int PAD    = 20;
    constexpr int GAP    = 12;
    const int bar_h = (m_height - PAD * 2 - GAP * 3) / 4;
    const int bar_w = m_width - PAD * 2;

    auto y = [&](int row) { return PAD + row * (bar_h + GAP); };

    m_gauges.push_back(std::make_unique<RpmGauge>  (SDL_Rect{PAD, y(0), bar_w, bar_h}));
    m_gauges.push_back(std::make_unique<SpeedGauge>(SDL_Rect{PAD, y(1), bar_w, bar_h}));
    m_gauges.push_back(std::make_unique<FuelGauge> (SDL_Rect{PAD, y(2), bar_w, bar_h}));
    m_gauges.push_back(std::make_unique<TempGauge> (SDL_Rect{PAD, y(3), bar_w, bar_h}));
}

void Dashboard::update(const DashboardData& data) {
    handle_events();
    m_gauges[0]->set_value(data.rpm);
    m_gauges[1]->set_value(data.speed_kmh);
    m_gauges[2]->set_value(data.fuel_percent);
    m_gauges[3]->set_value(data.coolant_temp_c);
}

void Dashboard::render() {
    SDL_SetRenderDrawColor(m_renderer, 0, 0, 0, 255);
    SDL_RenderClear(m_renderer);

    for (auto& gauge : m_gauges)
        gauge->render(m_renderer);

    SDL_RenderPresent(m_renderer);
}

void Dashboard::handle_events() {
    SDL_Event e;
    while (SDL_PollEvent(&e)) {
        if (e.type == SDL_QUIT)
            m_running = false;
        if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_ESCAPE)
            m_running = false;
    }
}
