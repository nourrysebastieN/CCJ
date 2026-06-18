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

    m_renderer = SDL_CreateRenderer(m_window, -1,
                                    SDL_RENDERER_ACCELERATED | SDL_RENDERER_TARGETTEXTURE);
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
        if (!slot.enabled) {
            m_gauges.push_back(nullptr);   // keep index aligned with m_layout
            continue;
        }
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
            case GaugeType::RPM:           value = data.rpm;                         break;
            case GaugeType::SPEED:         value = data.speed_kmh;                   break;
            case GaugeType::FUEL:          value = data.fuel_percent;                 break;
            case GaugeType::COOLANT_TEMP:  value = data.coolant_temp_c;              break;
            case GaugeType::BLINKER_LEFT:  value = data.blinker_left  ? 1.0f : 0.0f; break;
            case GaugeType::BLINKER_RIGHT: value = data.blinker_right ? 1.0f : 0.0f; break;
            case GaugeType::WARNING_CEL:   value = data.check_engine  ? 1.0f : 0.0f; break;
            case GaugeType::WARNING_OIL:   value = data.warning_oil   ? 1.0f : 0.0f; break;
            case GaugeType::WARNING_BAT:       value = data.warning_bat    ? 1.0f : 0.0f; break;
            case GaugeType::WARNING_HAZARD:   value = data.warning_hazard ? 1.0f : 0.0f; break;
            case GaugeType::WARNING_LOW_BEAM: value = data.low_beam       ? 1.0f : 0.0f; break;
            case GaugeType::WARNING_HIGH_BEAM:value = data.high_beam      ? 1.0f : 0.0f; break;
        }
        if (m_gauges[i])
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
        if (gauge) gauge->render(m_renderer);

    if (m_show_grid)
        draw_grid();

    SDL_RenderPresent(m_renderer);
}

void Dashboard::handle_events() {
    SDL_Event e;
    while (SDL_PollEvent(&e)) {
        if (e.type == SDL_QUIT)
            m_running = false;
        if (e.type == SDL_KEYDOWN) {
            switch (e.key.keysym.sym) {
                case SDLK_ESCAPE: m_running   = false;         break;
                case SDLK_g:      m_show_grid = !m_show_grid;  break;
            }
        }
    }
}

void Dashboard::draw_grid() const {
    SDL_SetRenderDrawBlendMode(m_renderer, SDL_BLENDMODE_BLEND);

    // Minor lines every 10 px
    SDL_SetRenderDrawColor(m_renderer, 255, 255, 255, 28);
    for (int x = 0; x < m_width;  x += 10)
        SDL_RenderDrawLine(m_renderer, x, 0, x, m_height);
    for (int y = 0; y < m_height; y += 10)
        SDL_RenderDrawLine(m_renderer, 0, y, m_width, y);

    // Major lines every 100 px — overdraw minor lines with higher alpha
    SDL_SetRenderDrawColor(m_renderer, 255, 255, 255, 80);
    for (int x = 0; x < m_width;  x += 100)
        SDL_RenderDrawLine(m_renderer, x, 0, x, m_height);
    for (int y = 0; y < m_height; y += 100)
        SDL_RenderDrawLine(m_renderer, 0, y, m_width, y);

    // Slot bounding boxes: red = enabled, grey = disabled.
    // The gauge arc is centered inside this rect and won't reach the edges
    // unless w == h (square bounds).
    for (const auto& slot : m_layout) {
        if (slot.enabled)
            SDL_SetRenderDrawColor(m_renderer, 255, 80, 80, 220);
        else
            SDL_SetRenderDrawColor(m_renderer, 140, 140, 140, 140);

        SDL_RenderDrawRect(m_renderer, &slot.bounds);
        SDL_Rect inner{ slot.bounds.x + 1, slot.bounds.y + 1,
                        slot.bounds.w - 2, slot.bounds.h - 2 };
        SDL_RenderDrawRect(m_renderer, &inner);
    }

    SDL_SetRenderDrawBlendMode(m_renderer, SDL_BLENDMODE_NONE);
}
