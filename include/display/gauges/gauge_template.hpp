#pragma once

#include <SDL2/SDL.h>
#include <string>
#include <vector>

// A value range over which a specific color is drawn on the arc.
struct ColorZone {
    float     from;
    float     to;
    SDL_Color color;
};

// Describes the complete visual appearance of one analog gauge.
// All drawing is done with SDL2 primitives — no PNG required.
struct GaugeTemplate {
    float min_value   =   0.0f;
    float max_value   = 100.0f;

    // Arc sweep: start_angle is where min_value sits (degrees CW from 12 o'clock).
    // The needle sweeps clockwise by `sweep` degrees to reach max_value.
    float start_angle = 225.0f;   // bottom-left  (≈ 7 o'clock)
    float sweep       = 270.0f;   // ends bottom-right (≈ 5 o'clock)

    SDL_Color arc_color    {180, 180, 180, 255};   // base scale track
    SDL_Color tick_color   {220, 220, 220, 255};   // tick marks
    SDL_Color needle_color { 30, 200,  30, 255};   // needle

    std::vector<ColorZone> zones;   // colored overlays on the arc (e.g. redline)
    int major_ticks = 9;            // number of tick marks drawn on the scale

    // Optional path to an SVG face image. When set, the SVG is rasterised to
    // the gauge bounds and replaces the programmatic arc/zones/ticks drawing.
    // The needle is always drawn on top regardless.
    // Leave empty (default) to use the programmatic drawing.
    std::string face_svg;
};
