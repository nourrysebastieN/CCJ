# CCJ Dashboard Assets

Gauge faces are drawn entirely with SDL2 primitives — **no PNG files are needed
for individual gauges**. The only asset each theme requires is a background image
that fills the full dashboard window.

## Directory Structure

```
assets/
└── themes/
    ├── default/
    │   └── dashboard_bg.png    ← the only required file per theme
    └── carbon/
        └── dashboard_bg.png
```

## Switching Themes

```cpp
// src/main.cpp — one line to change the theme
const Theme theme = Theme::from("assets/themes", "carbon");
```

## `dashboard_bg.png`

Rendered behind all gauges, stretched to fill the dashboard window.
Must match the window size passed to `Dashboard(width, height, ...)`.

| Window       | `dashboard_bg.png` size |
|--------------|-------------------------|
| 1280 × 480   | **1280 × 480 px**       |
| 1280 × 1024  | **1280 × 1024 px**      |
| 1588 × 1076  | **1588 × 1076 px**      |

Missing or absent → black background. The dashboard always runs without it.

## Gauge Appearance

Gauges are configured via `GaugeTemplate` in code (`AnalogGaugeFactory::create()`).
Each gauge type has its own template with:

| Gauge        | Range       | Color zones                              | Ticks |
|--------------|-------------|------------------------------------------|-------|
| RPM          | 0 – 8000    | Red: 6500 – 8000 (redline)              | 8     |
| Speed        | 0 – 220     | —                                        | 11    |
| Fuel         | 0 – 100 %   | Yellow: 0 – 15 % (low-fuel warning)     | 4     |
| Coolant temp | 20 – 120 °C | Blue: 20 – 60 °C · Red: 105 – 120 °C   | 5     |

To change a gauge's look, edit its `GaugeTemplate` in
`src/display/gauges/analog_gauge_factory.cpp`.

## Gauge Drawing

`AnalogGauge::render()` draws three layers in order:

1. **Scale arc** — grey annular band from `min` to `max`
2. **Color zones** — colored overlays on the arc (redline, warnings)
3. **Tick marks** — evenly-spaced radial lines on the arc
4. **Needle** — 3-pixel-wide line pointing at the current value

All dimensions (arc thickness, needle length, tick size) scale proportionally
with the gauge `SDL_Rect` bounds — no fixed pixel sizes.
