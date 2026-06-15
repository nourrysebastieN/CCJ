#pragma once

#include "signals/signal_processor.h"

#include <SDL2/SDL.h>
#include <memory>
#include <vector>

class Gauge;

class Dashboard {
public:
    Dashboard(int width, int height);
    ~Dashboard();

    bool init();
    void update(const DashboardData& data);
    void render();
    bool is_running() const { return m_running; }

private:
    int m_width;
    int m_height;
    bool m_running{true};

    SDL_Window*   m_window{nullptr};
    SDL_Renderer* m_renderer{nullptr};

    std::vector<std::unique_ptr<Gauge>> m_gauges;

    void handle_events();
    void build_layout();
};
