#include "display/dashboard.hpp"

#include "display/gauges/gauge_factory.hpp"

#ifdef CCJ_HAS_SDL2_IMAGE
#include <SDL2/SDL_image.h>
#endif

Dashboard::Dashboard(int width, int height,
                     std::unique_ptr<IGaugeFactory> factory,
                     std::vector<GaugeSlot>         layout)
    : m_width(width), m_height(height)
    , m_factory(std::move(factory))
    , m_layout(std::move(layout)) {}

Dashboard::~Dashboard() {
    if (m_bg_tex)   SDL_DestroyTexture(m_bg_tex);
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

    load_background();
    build_gauges();
    return true;
}

void Dashboard::load_background() {
#ifdef CCJ_HAS_SDL2_IMAGE
    const std::string bg = m_factory->background_path();
    if (bg.empty()) return;
    SDL_Surface* surf = IMG_Load(bg.c_str());
    if (!surf) return;
    m_bg_tex = SDL_CreateTextureFromSurface(m_renderer, surf);
    SDL_FreeSurface(surf);
#endif
}

void Dashboard::build_gauges() {
    m_gauges.clear();
    for (const auto& slot : m_layout) {
        auto gauge = m_factory->create(slot.type, slot.bounds);
        gauge->load_assets(m_renderer);
        m_gauges.push_back(std::move(gauge));
    }
}

void Dashboard::update(const DashboardData& data) {
    handle_events();
    for (size_t i = 0; i < m_layout.size(); ++i) {
        float value = 0.0f;
        switch (m_layout[i].type) {
            case GaugeType::RPM:          value = data.rpm;            break;
            case GaugeType::SPEED:        value = data.speed_kmh;      break;
            case GaugeType::FUEL:         value = data.fuel_percent;    break;
            case GaugeType::COOLANT_TEMP: value = data.coolant_temp_c; break;
        }
        m_gauges[i]->set_value(value);
    }
}

void Dashboard::render() {
    SDL_SetRenderDrawColor(m_renderer, 0, 0, 0, 255);
    SDL_RenderClear(m_renderer);

    if (m_bg_tex) {
        SDL_Rect full{0, 0, m_width, m_height};
        SDL_RenderCopy(m_renderer, m_bg_tex, nullptr, &full);
    }

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
