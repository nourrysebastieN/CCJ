# CCJ — Architecture Documentation

## Table of Contents

1. [Project Overview](#1-project-overview)
2. [Directory Structure](#2-directory-structure)
3. [Signal Pipeline](#3-signal-pipeline)
4. [Display System](#4-display-system)
5. [Abstract Factory Pattern](#5-abstract-factory-pattern)
6. [Layout System](#6-layout-system)
7. [Asset Loading](#7-asset-loading)
8. [Simulation Mode](#8-simulation-mode)
9. [Adding a New Gauge Type](#9-adding-a-new-gauge-type)
10. [Adding a New Dashboard Theme](#10-adding-a-new-dashboard-theme)

---

## 1. Project Overview

CCJ is a C++17 digital instrument cluster for a Honda Civic EK (6th gen, 1996–2000) running on a Raspberry Pi. It reads original OEM sensor signals — ignition pulses, VSS pulses, and resistive senders — and renders a real-time dashboard via SDL2.

```
Car sensors (12V) → Raspberry Pi → Signal reading → Conversion → SDL2 render
```

A compile-time flag (`-DCCJ_SIMULATE=ON`) replaces all hardware I/O with a software simulator, enabling development and testing on any desktop machine.

---

## 2. Directory Structure

```
ccj/
├── include/                    # All header files (.hpp)
│   ├── signals/                # Hardware input layer
│   │   ├── pulse_reader.hpp    # GPIO interrupt counter for RPM + VSS
│   │   ├── adc_reader.hpp      # SPI/ADC reader for fuel + coolant temp
│   │   ├── signal_processor.hpp# Raw values → physical units
│   │   └── simulator.hpp       # Software signal source (no hardware)
│   └── display/                # Rendering layer
│       ├── dashboard.hpp       # SDL2 window, event loop, render orchestration
│       ├── layout.hpp          # GaugeSlot + predefined layout builders
│       └── gauges/
│           ├── gauge.hpp           # Abstract Gauge base class
│           ├── gauge_factory.hpp   # GaugeType enum + IGaugeFactory interface
│           ├── bar_gauge_factory.hpp
│           ├── analog_gauge_factory.hpp
│           ├── analog_gauge.hpp
│           ├── rpm_gauge.hpp
│           ├── speed_gauge.hpp
│           ├── fuel_gauge.hpp
│           └── temp_gauge.hpp
├── src/                        # All implementation files (.cpp)
│   ├── main.cpp
│   ├── signals/
│   └── display/
│       └── gauges/
├── assets/                     # PNG images for analog theme
├── config/
│   └── ek_calibration.json     # Sensor calibration curves
└── CMakeLists.txt
```

**Convention:** `.cpp` files use absolute include paths from the `include/` root
(e.g. `"display/gauges/rpm_gauge.hpp"`). Headers use relative paths within their
own module (e.g. `"gauge.hpp"` inside `include/display/gauges/`).

---

## 3. Signal Pipeline

```
           ┌──────────────┐     ┌──────────────────┐
GPIO 17 ──►│  PulseReader │────►│                  │
GPIO 27 ──►│  (RPM, VSS)  │     │  SignalProcessor  │──► DashboardData
SPI ADC ──►│  AdcReader   │────►│  (calibration)   │
           │  (fuel, temp)│     │                  │
           └──────────────┘     └──────────────────┘
```

### `PulseReader`
Installs GPIO interrupt handlers on two pins. A background thread polls the
atomic pulse counters every 100 ms and converts them to RPM and km/h using the
EK calibration constants (1 pulse/rev, 4000 pulses/km).

### `AdcReader`
Opens an SPI channel to an MCP3008 ADC. `read_fuel_raw()` and `read_temp_raw()`
return 10-bit counts (0–1023) representing the voltage across a voltage divider.

### `SignalProcessor`
Converts raw ADC counts to human-readable units:

- **Fuel %** — voltage divider formula → sender resistance (Ω) → percent fill.
- **Coolant °C** — voltage divider → thermistor resistance → Steinhart-Hart NTC
  equation → Kelvin → Celsius.

All calibration constants (resistances, reference voltage, Steinhart-Hart
coefficients) live in `config/ek_calibration.json`.

### `DashboardData`
The output struct passed downstream to the display layer every frame:

```cpp
struct DashboardData {
    float rpm;
    float speed_kmh;
    float fuel_percent;
    float coolant_temp_c;
};
```

---

## 4. Display System

```
Dashboard::init()
  └── load_background()      ← loads dashboard_bg.png if factory provides one
  └── build_gauges()         ← calls IGaugeFactory::create() for each GaugeSlot

Dashboard::update(DashboardData)
  └── for each slot: route data.field → gauge[i]->set_value()

Dashboard::render()
  └── SDL_RenderClear
  └── SDL_RenderCopy(m_bg_tex)          ← optional full-window background
  └── for each gauge: gauge->render()   ← bar fill or needle rotation
  └── SDL_RenderPresent
```

`Dashboard` owns the `SDL_Window`, `SDL_Renderer`, the background texture, and
the vector of instantiated gauges. It has no knowledge of concrete gauge types —
all gauge creation is delegated to `IGaugeFactory`.

---

## 5. Abstract Factory Pattern

This is the core design pattern of the rendering layer. It solves two problems:

1. **Swapping the entire visual theme** (bar gauges ↔ analog needles) in one
   line without touching `Dashboard` or any other code.
2. **Ensuring a gauge family is internally consistent** — the factory guarantees
   that all gauges, the needle sprite, and the background image belong to the
   same theme.

### Class Hierarchy

```
IGaugeFactory                          (include/display/gauges/gauge_factory.hpp)
│   + create(GaugeType, SDL_Rect) = 0
│   + background_path() → ""
│
├── BarGaugeFactory                    (include/display/gauges/bar_gauge_factory.hpp)
│   + create()  → RpmGauge | SpeedGauge | FuelGauge | TempGauge
│   + background_path() → "assets/dashboard_bg.png"  (or "" if no assets_dir)
│
└── AnalogGaugeFactory                 (include/display/gauges/analog_gauge_factory.hpp)
    + create()  → AnalogGauge (min/max per type, bg + needle paths)
    + background_path() → "assets/dashboard_bg.png"


Gauge                                  (include/display/gauges/gauge.hpp)
│   + render(SDL_Renderer*) = 0
│   + set_value(float) = 0
│   + load_assets(SDL_Renderer*)       ← default no-op
│
├── RpmGauge      — horizontal fill bar, 0–8000, redline marker at 6500
├── SpeedGauge    — horizontal fill bar, 0–220 km/h, green→white gradient
├── FuelGauge     — horizontal fill bar, 0–100%, yellow warning at ≤15%
├── TempGauge     — horizontal fill bar, 20–120°C, blue/white/red zones
└── AnalogGauge   — texture background + rotating needle (SDL_RenderCopyEx)
```

### The `IGaugeFactory` Interface

```cpp
// include/display/gauges/gauge_factory.hpp

enum class GaugeType { RPM, SPEED, FUEL, COOLANT_TEMP };

class IGaugeFactory {
public:
    virtual ~IGaugeFactory() = default;
    virtual std::unique_ptr<Gauge> create(GaugeType type, SDL_Rect bounds) = 0;
    virtual std::string background_path() const { return ""; }
};
```

`create()` is the factory method. Given a logical type and a bounding rectangle,
it returns the appropriate concrete `Gauge` ready to render. `background_path()`
lets the factory declare which full-window background image goes with its theme.

### How `Dashboard` Uses the Factory

`Dashboard` receives a factory at construction time and never sees concrete gauge
classes again:

```cpp
// src/display/dashboard.cpp

void Dashboard::build_gauges() {
    for (const auto& slot : m_layout) {
        auto gauge = m_factory->create(slot.type, slot.bounds);
        gauge->load_assets(m_renderer);          // no-op for bar gauges
        m_gauges.push_back(std::move(gauge));
    }
}
```

The `update()` method routes sensor data by `GaugeType`, so gauge order in the
layout does not affect which data each gauge receives:

```cpp
void Dashboard::update(const DashboardData& data) {
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
```

### Switching Themes — One Line in `main.cpp`

```cpp
// Bar-fill gauges, stacked layout
Dashboard dashboard(1280, 480,
    std::make_unique<BarGaugeFactory>("assets"),
    Layout::stacked(1280, 480));

// Analog needle gauges, 2×2 grid
Dashboard dashboard(1280, 480,
    std::make_unique<AnalogGaugeFactory>("assets"),
    Layout::grid_2x2(1280, 480));
```

The factory and the layout are independent choices — any factory can be combined
with any layout.

### `AnalogGauge` — Texture + Needle Rotation

`AnalogGauge` is the concrete product of `AnalogGaugeFactory`. On `load_assets()`
it loads two PNG textures via SDL2_image:

- **Background texture** (`rpm_bg.png`, `speed_bg.png`, …) — the static gauge
  face rendered with `SDL_RenderCopy`.
- **Needle texture** (`needle.png`, shared by all gauges) — rotated per-frame
  with `SDL_RenderCopyEx`.

Needle angle mapping (270° clockwise sweep):

```
value = min  →  angle = 225°  (7–8 o'clock, bottom-left)
value = max  →  angle = 495°  (4–5 o'clock, bottom-right)

angle = 225.0f + (value - min) / (max - min) * 270.0f
```

The needle image must point **straight up (12 o'clock)** and be centered on its
canvas. `SDL_RenderCopyEx` rotates it clockwise around the center of the gauge
bounds.

If either texture is missing, `AnalogGauge::render()` returns immediately and
draws nothing — the window area stays transparent to whatever is underneath
(the dashboard background, or black).

---

## 6. Layout System

### What a Layout Is

A layout is a `std::vector<GaugeSlot>`. Each slot maps one logical gauge type
to a pixel rectangle inside the SDL2 window:

```cpp
// include/display/layout.hpp

struct GaugeSlot {
    GaugeType type;    // which sensor value this gauge displays
    SDL_Rect  bounds;  // { x, y, width, height } in window pixels
                       // SDL_RenderCopy/Ex stretches the gauge texture to fill this rect
};
```

`bounds` is the exact rectangle passed to the gauge's `render()` call.
For bar gauges it defines the fill area. For analog gauges it defines where
the background texture is drawn and the rectangle within which `SDL_RenderCopyEx`
rotates the needle — so **the aspect ratio of `bounds` directly affects how
the gauge looks**.

### How `Dashboard` Consumes the Layout

```
Dashboard::build_gauges()
  for each GaugeSlot:
    gauge = factory->create(slot.type, slot.bounds)   ← bounds baked in at creation
    gauge->load_assets(renderer)

Dashboard::update(DashboardData)
  for each slot[i]:
    route data field → gauges[i]->set_value()         ← matched by GaugeType, not index
```

Because data routing uses `GaugeType`, not vector index, gauge order in the
layout vector does not affect which value each gauge receives.

### Predefined Layouts (`src/display/layout.cpp`)

#### `Layout::stacked(width, height)`

```
┌────────────────────────────────────────┐  ↑ PAD
│  RPM  ██████████████░░░░░░░░░░░░░░░░  │  ↕ bar_h
├────────────────────────────────────────┤  ↕ GAP
│  SPD  █████████░░░░░░░░░░░░░░░░░░░░░  │  ↕ bar_h
├────────────────────────────────────────┤  ↕ GAP
│  FUEL ████████████████░░░░░░░░░░░░░░  │  ↕ bar_h
├────────────────────────────────────────┤  ↕ GAP
│  TEMP ████████░░░░░░░░░░░░░░░░░░░░░░  │  ↕ bar_h
└────────────────────────────────────────┘  ↓ PAD
 ←PAD→←────────── bar_w ──────────────→←PAD→
```

```
PAD = 20 px,  GAP = 12 px
bar_w = width  − PAD×2
bar_h = (height − PAD×2 − GAP×3) / 4
```

At **1280×480**: each bar is **1240 × 101 px**.
Best with `BarGaugeFactory` — bar images should be 1240×101.

---

#### `Layout::grid_2x2(width, height)`

```
┌────────────────────┬────────────────────┐
│                    │                    │
│  RPM               │  SPEED             │  cell_h
│                    │                    │
├────────────────────┼────────────────────┤
│                    │                    │
│  FUEL              │  TEMP              │  cell_h
│                    │                    │
└────────────────────┴────────────────────┘
 ←PAD→← cell_w →←GAP→← cell_w →←PAD→
```

```
PAD = 20 px,  GAP = 12 px
cell_w = (width  − PAD×2 − GAP) / 2
cell_h = (height − PAD×2 − GAP) / 2
```

At **1280×480**: each cell is **614 × 214 px** (ratio ≈ 2.9:1).
Gauge images are stretched to fill the rectangular cell. Circular dials
become ellipses — use `analog_grid_2x2` instead for analog needles.

---

#### `Layout::analog_grid_2x2(width, height)`

Same 2×2 grid, but each gauge bounds is a **square** (side = `cell_h`)
centered horizontally within its cell. This prevents circular dial textures
and the rotating needle from being distorted.

```
┌───────────────────────┬───────────────────────┐
│   ←dx→┌────────┐      │   ←dx→┌────────┐      │
│       │  RPM   │      │       │ SPEED  │      │
│       └────────┘      │       └────────┘      │
├───────────────────────┼───────────────────────┤
│   ←dx→┌────────┐      │   ←dx→┌────────┐      │
│       │  FUEL  │      │       │  TEMP  │      │
│       └────────┘      │       └────────┘      │
└───────────────────────┴───────────────────────┘
       ← dial →                ← dial →
```

```
dial = cell_h               (square side)
dx   = (cell_w − dial) / 2  (left margin inside cell)
```

At **1280×480**: each dial is **214 × 214 px**, centered within a 614 px wide cell.
Use with `AnalogGaugeFactory` — gauge face and needle images should be **214×214 px**.

---

### Pixel Dimensions Summary

| Layout | Window | Gauge cell size | Dashboard bg |
|---|---|---|---|
| `stacked` | 1280 × 480 | 1240 × 101 px | 1280 × 480 px |
| `grid_2x2` | **1588 × 1076** | **768 × 512 px** (3:2) | 1588 × 1076 px |
| `analog_grid_2x2` | 1280 × 1024 | 486 × 486 px (1:1) | 1280 × 1024 px |

The `grid_2x2` window (1588×1076) is sized so cells land exactly at **768×512**,
which is the native gauge image size (1536×1024) scaled to 50% with no distortion.

### Custom Layouts

A fully custom layout can be built inline — `Dashboard` imposes no constraints
on gauge count, position, or size:

```cpp
std::vector<GaugeSlot> layout = {
    { GaugeType::RPM,          {  20,  20, 600, 440 } },  // large tachometer, left
    { GaugeType::SPEED,        { 660,  20, 600, 210 } },  // speedometer, top-right
    { GaugeType::FUEL,         { 660, 250, 290, 210 } },  // fuel, bottom-right
    { GaugeType::COOLANT_TEMP, { 970, 250, 290, 210 } },  // temp, bottom-far-right
};
```

---

## 7. Asset Loading

Asset loading is optional and gated behind the `CCJ_HAS_SDL2_IMAGE` compile
definition, which CMake sets automatically when `libsdl2-image-dev` is present.

```
Install:  sudo apt-get install libsdl2-image-dev
Rebuild:  cmake -S . -B build && cmake --build build
```

### Expected files under `assets/`

| File | Used by | Native size | Displayed at (`grid_2x2`) | Content |
|---|---|---|---|---|
| `dashboard_bg.png` | `Dashboard` | any | 1588 × 1076 px | Full-window background |
| `rpm_bg.png` | `AnalogGauge` (RPM) | 1536 × 1024 px | 768 × 512 px | Tachometer face |
| `speed_bg.png` | `AnalogGauge` (Speed) | 1536 × 1024 px | 768 × 512 px | Speedometer face |
| `fuel_bg.png` | `AnalogGauge` (Fuel) | 1536 × 1024 px | 768 × 512 px | Fuel gauge face |
| `temp_bg.png` | `AnalogGauge` (Coolant) | 1536 × 1024 px | 768 × 512 px | Temperature gauge face |
| `needle.png` | All `AnalogGauge` instances | 1536 × 1024 px | 768 × 512 px | Needle pointing up, rotation axis at center |

### Loading sequence

```
Dashboard::init()
  └── load_background()
        factory->background_path() → "assets/dashboard_bg.png"
        IMG_Load → SDL_CreateTextureFromSurface → m_bg_tex

  └── build_gauges()
        for each slot:
          gauge = factory->create(type, bounds)
          gauge->load_assets(renderer)
            IMG_Load(bg_path)     → m_bg_tex
            IMG_Load(needle_path) → m_needle_tex
```

---

## 8. Simulation Mode

Build with `-DCCJ_SIMULATE=ON` (CMake option). The preprocessor gate in
`main.cpp` replaces `PulseReader`, `AdcReader`, and `SignalProcessor` with a
`Simulator` instance that generates a 30-second scripted drive cycle:

| Time | RPM | Speed | Notes |
|---|---|---|---|
| 0–5 s | ~1200 ± jitter | 0 km/h | Cold idle |
| 5–15 s | 0 → 6500 linear | 0 → 100 km/h | Rev-up, acceleration |
| 15–30 s | 2500–5500 sawtooth | ~80 km/h | Gear shift cycle |

Fuel drains cosmetically at 0.12 %/min. Coolant warms exponentially from 20 °C
to 90 °C over 300 seconds. The cycle loops.

---

## 9. Adding a New Gauge Type

**Example: oil pressure gauge.**

### Step 1 — Add the enum value

```cpp
// include/display/gauges/gauge_factory.hpp
enum class GaugeType {
    RPM, SPEED, FUEL, COOLANT_TEMP,
    OIL_PRESSURE,   // ← add here
};
```

### Step 2 — Create the gauge class

```cpp
// include/display/gauges/oil_gauge.hpp
#pragma once
#include "gauge.hpp"

class OilGauge : public Gauge {
public:
    explicit OilGauge(SDL_Rect bounds);
    void set_value(float value) override;   // value in bar (0–10)
    void render(SDL_Renderer* renderer) override;
private:
    float m_value{0.0f};
};
```

### Step 3 — Register in each factory

```cpp
// src/display/gauges/gauge_factory.cpp  (BarGaugeFactory)
case GaugeType::OIL_PRESSURE: return std::make_unique<OilGauge>(bounds);

// src/display/gauges/analog_gauge_factory.cpp
case GaugeType::OIL_PRESSURE:
    return std::make_unique<AnalogGauge>(bounds, 0.0f, 10.0f,
                                         path("oil_bg.png"), needle);
```

### Step 4 — Wire the data in `Dashboard::update()`

```cpp
// src/display/dashboard.cpp
case GaugeType::OIL_PRESSURE: value = data.oil_pressure_bar; break;
```

### Step 5 — Add to your layout

```cpp
layout.push_back({ GaugeType::OIL_PRESSURE, { x, y, w, h } });
```

`Dashboard` requires no other changes.

---

## 10. Adding a New Dashboard Theme

**Example: a minimal HUD theme with white-on-black bars and no decorations.**

### Step 1 — Create the factory

```cpp
// include/display/gauges/hud_gauge_factory.hpp
#pragma once
#include "gauge_factory.hpp"

class HudGaugeFactory : public IGaugeFactory {
public:
    std::unique_ptr<Gauge> create(GaugeType type, SDL_Rect bounds) override;
    // background_path() intentionally omitted → defaults to "" (no background)
};
```

### Step 2 — Implement gauge creation

```cpp
// src/display/gauges/hud_gauge_factory.cpp
#include "display/gauges/hud_gauge_factory.hpp"
#include "display/gauges/hud_gauge.hpp"   // your new minimal gauge widget

std::unique_ptr<Gauge> HudGaugeFactory::create(GaugeType type, SDL_Rect bounds) {
    switch (type) {
        case GaugeType::RPM:          return std::make_unique<HudGauge>(bounds, 0.f, 8000.f);
        case GaugeType::SPEED:        return std::make_unique<HudGauge>(bounds, 0.f,  220.f);
        case GaugeType::FUEL:         return std::make_unique<HudGauge>(bounds, 0.f,  100.f);
        case GaugeType::COOLANT_TEMP: return std::make_unique<HudGauge>(bounds, 20.f, 120.f);
    }
    return nullptr;
}
```

### Step 3 — Select in `main.cpp`

```cpp
Dashboard dashboard(1280, 480,
    std::make_unique<HudGaugeFactory>(),
    Layout::stacked(1280, 480));
```

Zero changes to `Dashboard`, `Layout`, or any existing gauge class.
