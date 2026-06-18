#!/usr/bin/env python3
"""
Generate SVG gauge faces that exactly mirror AnalogGauge's programmatic drawing.

Angle convention (same as analog_gauge.cpp):
  0° = 12 o'clock, increases clockwise.
  x = cx + r * sin(θ)
  y = cy - r * cos(θ)

Run from the project root:
  python3 scripts/gen_gauge_svg.py
"""

import math
import os

PI = math.pi
CANVAS = 500          # SVG viewport (scaled to bounds at runtime)


# ---------------------------------------------------------------------------
# Geometry helpers
# ---------------------------------------------------------------------------

def pt(cx, cy, r, angle_deg):
    """Cartesian point for an angle (CW from 12 o'clock)."""
    rad = angle_deg * PI / 180.0
    return (cx + r * math.sin(rad), cy - r * math.cos(rad))


def angle_for(value, min_val, max_val, start_angle, sweep):
    ratio = (value - min_val) / (max_val - min_val)
    return start_angle + ratio * sweep


def arc_path(cx, cy, inner_r, outer_r, from_angle, to_angle):
    """
    SVG path for a filled annular arc sector (from_angle → to_angle, CW).
    Returns a 'd' attribute string.
    """
    span = abs(to_angle - from_angle)
    large = 1 if span > 180 else 0

    p_is = pt(cx, cy, inner_r, from_angle)
    p_ie = pt(cx, cy, inner_r, to_angle)
    p_oe = pt(cx, cy, outer_r, to_angle)
    p_os = pt(cx, cy, outer_r, from_angle)

    fmt = lambda p: f"{p[0]:.2f},{p[1]:.2f}"
    ir, or_ = f"{inner_r:.2f}", f"{outer_r:.2f}"

    return (
        f"M {fmt(p_is)} "
        f"A {ir},{ir} 0 {large},1 {fmt(p_ie)} "   # CW  (sweep=1)
        f"L {fmt(p_oe)} "
        f"A {or_},{or_} 0 {large},0 {fmt(p_os)} " # CCW (sweep=0)
        f"Z"
    )


def tick_element(cx, cy, inner_r, outer_r, angle_deg, stroke, width):
    p1 = pt(cx, cy, inner_r, angle_deg)
    p2 = pt(cx, cy, outer_r, angle_deg)
    return (
        f'<line x1="{p1[0]:.2f}" y1="{p1[1]:.2f}" '
        f'x2="{p2[0]:.2f}" y2="{p2[1]:.2f}" '
        f'stroke="{stroke}" stroke-width="{width}" stroke-linecap="round"/>'
    )


def rgb(r, g, b, a=None):
    if a is not None:
        return f"rgba({r},{g},{b},{a})"
    return f"rgb({r},{g},{b})"


# ---------------------------------------------------------------------------
# SVG generator
# ---------------------------------------------------------------------------

def generate(name, min_val, max_val, start_angle, sweep,
             major_ticks, arc_color, tick_color, zones,
             size=CANVAS, bg_opacity=0.8):
    """
    bg_opacity: 0.0 = fully transparent face (background shows through),
                1.0 = solid dark face.  Values in between give a tinted overlay.
    """
    cx = cy = size / 2.0
    r         = size / 2.0 - 4.0
    arc_outer  = r * 0.90
    arc_inner  = r * 0.78
    tick_outer = r * 0.90
    tick_inner = r * 0.70
    end_angle  = start_angle + sweep

    el = []

    # ── Background (skip element entirely when fully transparent)
    if bg_opacity > 0.0:
        el.append(
            f'<circle cx="{cx}" cy="{cy}" r="{r:.2f}" '
            f'fill="rgb(15,15,20)" fill-opacity="{bg_opacity:.2f}"/>'
        )

    # ── Outer bezel ring (always drawn, gives the gauge a clean edge)
    el.append(
        f'<circle cx="{cx}" cy="{cy}" r="{r:.2f}" '
        f'fill="none" stroke="rgb(55,55,65)" stroke-width="3"/>'
    )

    # ── Base arc track
    d = arc_path(cx, cy, arc_inner, arc_outer, start_angle, end_angle)
    el.append(f'<path d="{d}" fill="{rgb(*arc_color)}"/>')

    # ── Color zones (overlay on the track)
    for z_from, z_to, z_col in zones:
        a0 = angle_for(z_from, min_val, max_val, start_angle, sweep)
        a1 = angle_for(z_to,   min_val, max_val, start_angle, sweep)
        d  = arc_path(cx, cy, arc_inner, arc_outer, a0, a1)
        el.append(f'<path d="{d}" fill="{rgb(*z_col)}"/>')

    # ── Major ticks
    for i in range(major_ticks + 1):
        val   = min_val + i / major_ticks * (max_val - min_val)
        angle = angle_for(val, min_val, max_val, start_angle, sweep)
        el.append(tick_element(cx, cy, tick_inner, tick_outer,
                               angle, rgb(*tick_color), 2.5))

    # ── Minor ticks (4 subdivisions between each major tick)
    minor_per_major = 4
    for i in range(major_ticks * minor_per_major + 1):
        # skip positions that coincide with major ticks
        if i % minor_per_major == 0:
            continue
        val   = min_val + i / (major_ticks * minor_per_major) * (max_val - min_val)
        angle = angle_for(val, min_val, max_val, start_angle, sweep)
        inner = tick_inner + (tick_outer - tick_inner) * 0.45  # shorter tick
        el.append(tick_element(cx, cy, inner, tick_outer,
                               angle, rgb(*tick_color), 1.2))

    # ── Inner shadow ring (separates arc from face; hidden when transparent)
    if bg_opacity > 0.0:
        el.append(
            f'<circle cx="{cx}" cy="{cy}" r="{arc_inner:.2f}" '
            f'fill="none" stroke="rgb(8,8,12)" '
            f'stroke-opacity="{bg_opacity:.2f}" stroke-width="3"/>'
        )

    # ── Centre hub (needle pivot)
    hub_r = r * 0.06
    el.append(
        f'<circle cx="{cx}" cy="{cy}" r="{hub_r:.2f}" '
        f'fill="rgb(30,30,35)" stroke="rgb(90,90,100)" stroke-width="1.5"/>'
    )

    lines = [
        f'<svg xmlns="http://www.w3.org/2000/svg"',
        f'     viewBox="0 0 {size} {size}"',
        f'     width="{size}" height="{size}">',
        '',
    ] + ['  ' + e for e in el] + ['', '</svg>']

    return '\n'.join(lines)


# ---------------------------------------------------------------------------
# Gauge definitions  (must match config/default.json and AnalogGaugeFactory)
# ---------------------------------------------------------------------------

GAUGES = {
    'rpm': dict(
        min_val=0, max_val=8000,
        start_angle=225, sweep=270,
        major_ticks=8,
        arc_color=(180, 180, 180),
        tick_color=(220, 220, 220),
        zones=[
            (6500, 8000, (200, 30, 30)),
        ],
    ),
    'speed': dict(
        min_val=0, max_val=220,
        start_angle=225, sweep=270,
        major_ticks=11,
        arc_color=(180, 180, 180),
        tick_color=(220, 220, 220),
        zones=[],
    ),
    'fuel': dict(
        min_val=0, max_val=100,
        start_angle=225, sweep=270,
        major_ticks=4,
        arc_color=(180, 180, 180),
        tick_color=(220, 220, 220),
        zones=[
            (0, 15, (220, 180, 0)),
        ],
    ),
    'coolant_temp': dict(
        min_val=20, max_val=120,
        start_angle=225, sweep=270,
        major_ticks=5,
        arc_color=(180, 180, 180),
        tick_color=(220, 220, 220),
        zones=[
            (20,  60,  (30, 100, 200)),
            (105, 120, (200, 30,  30)),
        ],
    ),
}


if __name__ == '__main__':
    out_dir = os.path.join(os.path.dirname(__file__), '..', 'assets', 'gauges')
    os.makedirs(out_dir, exist_ok=True)

    for name, cfg in GAUGES.items():
        svg  = generate(name, **cfg)
        path = os.path.join(out_dir, f'{name}.svg')
        with open(path, 'w') as f:
            f.write(svg)
        print(f'  wrote {path}')

    print('Done.')
