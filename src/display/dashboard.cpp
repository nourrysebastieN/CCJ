#include "dashboard.h"

#include "gauges/gauge.h"
#include "gauges/rpm_gauge.h"

#include <stdexcept>

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

void Dashboard::build_layout() {
    // RPM bar — top quarter of screen
    SDL_Rect rpm_rect{20, 20, m_width - 40, m_height / 4 - 10};
    m_gauges.push_back(std::make_unique<RpmGauge>(rpm_rect));
}

void Dashboard::update(const DashboardData& data) {
    handle_events();
    m_gauges[0]->set_value(data.rpm); // RPM gauge
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
