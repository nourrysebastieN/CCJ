#!/usr/bin/env python3
"""
Generate white-on-transparent SVG warning symbol icons.

Each symbol uses only white strokes/fills on a transparent background so that
SDL_SetTextureColorMod(tex, r, g, b) can tint it to any configured color at
runtime without baking the color into the file.

Run from the project root:
  python3 scripts/gen_warning_svg.py
"""

import os

CANVAS = 200   # viewBox size (scaled to gauge bounds by SDL at load time)

# ---------------------------------------------------------------------------
# SVG templates
# ---------------------------------------------------------------------------

def wrap(elements, canvas=CANVAS):
    body = "\n".join(f"  {e}" for e in elements)
    return (
        f'<svg xmlns="http://www.w3.org/2000/svg"'
        f' viewBox="0 0 {canvas} {canvas}"'
        f' width="{canvas}" height="{canvas}">\n'
        f'{body}\n'
        f'</svg>\n'
    )


def path(d, fill="none", stroke="white", sw=10, extra=""):
    return (
        f'<path d="{d}" fill="{fill}" stroke="{stroke}"'
        f' stroke-width="{sw}" stroke-linecap="round" stroke-linejoin="round"'
        + (f' {extra}' if extra else '')
        + '/>'
    )


def rect(x, y, w, h, rx=0, fill="none", stroke="white", sw=9):
    return (
        f'<rect x="{x}" y="{y}" width="{w}" height="{h}" rx="{rx}"'
        f' fill="{fill}" stroke="{stroke}" stroke-width="{sw}"/>'
    )


def line(x1, y1, x2, y2, stroke="white", sw=9):
    return (
        f'<line x1="{x1}" y1="{y1}" x2="{x2}" y2="{y2}"'
        f' stroke="{stroke}" stroke-width="{sw}"'
        f' stroke-linecap="round"/>'
    )


# ---------------------------------------------------------------------------
# OIL PRESSURE symbol
# Oil can viewed from the side:
#   - Rectangular body with a small filler cap on top (centre-left)
#   - Horizontal spout exiting the left side of the body (mid-height)
#   - Downward nozzle at the spout tip
#   - Oil drop below the nozzle
# ---------------------------------------------------------------------------

def gen_oil():
    # Single closed path: traces the outer contour of the can body
    # (cap integrated into the path; spout opening cut into the left side).
    #
    # Reading order (clockwise from cap top-left):
    #   cap top → body top-right → right side → bottom → left side → spout journey → cap left (Z)
    body_d = (
        "M 100,30 "
        "L 140,30 L 140,50 "           # cap top and right side
        "L 175,50 Q 182,50 182,57 "    # body top-right corner
        "L 182,152 Q 182,160 174,160 " # body right side + bottom-right corner
        "L 82,160 Q 75,160 75,152 "    # body bottom + bottom-left corner
        "L 75,90 "                      # left side down to spout bottom
        "L 42,90 L 42,115 L 30,115 "   # spout bottom → nozzle right → nozzle cap
        "L 30,70 "                      # nozzle/spout left going up
        "L 75,70 "                      # spout top going right back to body
        "L 75,50 "                      # body left side continuing up
        "L 100,50 Z"                    # body top to cap left (Z closes cap left side)
    )

    # Teardrop drop below the nozzle tip
    drop_d = (
        "M 36,120 "
        "Q 19,133 19,145 Q 19,163 36,163 Q 53,163 53,145 Q 53,133 36,120 Z"
    )

    return wrap([
        path(body_d, sw=10),
        path(drop_d, fill="white", stroke="white", sw=1),
    ])


# ---------------------------------------------------------------------------
# BATTERY symbol
# Rectangle body with:
#   - Positive terminal (left, taller) and negative terminal (right, shorter) on top
#   - "+" sign inside left half, "−" inside right half
# ---------------------------------------------------------------------------

def gen_bat():
    return wrap([
        rect(15, 58, 170, 97, rx=8, sw=10),          # body
        rect(42, 40, 32, 20, rx=3, sw=8),             # positive terminal (taller)
        rect(126, 44, 30, 16, rx=3, sw=8),            # negative terminal
        line(38, 107, 78, 107, sw=9),                  # plus horizontal
        line(58, 87, 58, 127, sw=9),                   # plus vertical
        line(122, 107, 162, 107, sw=9),                # minus
    ])


# ---------------------------------------------------------------------------
# CHECK ENGINE LIGHT (CEL) symbol
# Simplified engine block viewed from the side:
#   - Main rectangular body with rounded corners
#   - Intake/valve-cover bump on the upper-right (extends above body)
#   - Oil-pan/exhaust bump on the lower-left (extends below body)
# ---------------------------------------------------------------------------

def gen_cel():
    # Single closed path traces the outer silhouette of the engine shape.
    d = (
        "M 40,55 "
        "L 85,55 L 85,30 Q 85,22 95,22 "       # body top → upper bump up
        "L 155,22 Q 163,22 163,30 "             # bump top → upper-right corner
        "L 163,55 "                              # bump down to body level
        "L 170,55 Q 178,55 178,63 "             # body top-right corner
        "L 178,128 Q 178,136 170,136 "          # right side + lower-right corner
        "L 115,136 L 115,160 Q 115,168 105,168 " # body bottom → lower bump down
        "L 45,168 Q 37,168 37,160 "             # bump bottom → lower-left corner
        "L 37,136 "                              # bump back up to body level
        "L 30,136 Q 22,136 22,128 "             # body bottom-left corner
        "L 22,63 Q 22,55 30,55 Z"              # left side + upper-left corner (close)
    )
    return wrap([path(d, sw=10)])


# ---------------------------------------------------------------------------
# HAZARD symbol
# Equilateral triangle (outline) with exclamation mark inside.
# ---------------------------------------------------------------------------

def gen_hazard():
    triangle = (
        '<polygon points="100,28 14,176 186,176" '
        'fill="none" stroke="white" stroke-width="11" '
        'stroke-linejoin="round" stroke-linecap="round"/>'
    )
    stem = line(100, 72, 100, 132, sw=14)
    dot  = '<circle cx="100" cy="155" r="10" fill="white"/>'
    return wrap([triangle, stem, dot])


# ---------------------------------------------------------------------------
# LOW BEAM symbol
# Headlight D-shape (right half of circle) + 4 rays angled ~20° downward.
# ---------------------------------------------------------------------------

def gen_low_beam():
    # Right-facing D: arc from (50,62) CW to (50,138), closed with vertical
    lens = (
        '<path d="M 50,62 A 38,38 0 0,1 50,138 Z" '
        'fill="white" stroke="none"/>'
    )
    # 4 parallel rays: start x=93, end x=185, Δy = +34 over 92px (≈20°)
    rays = [
        line(93, 68,  185, 102, sw=9),
        line(93, 90,  185, 124, sw=9),
        line(93, 112, 185, 146, sw=9),
        line(93, 134, 185, 168, sw=9),
    ]
    return wrap([lens] + rays)


# ---------------------------------------------------------------------------
# HIGH BEAM symbol
# Same headlight D-shape but 4 rays are horizontal.
# ---------------------------------------------------------------------------

def gen_high_beam():
    lens = (
        '<path d="M 50,62 A 38,38 0 0,1 50,138 Z" '
        'fill="white" stroke="none"/>'
    )
    rays = [
        line(93, 68,  185, 68,  sw=9),
        line(93, 90,  185, 90,  sw=9),
        line(93, 112, 185, 112, sw=9),
        line(93, 134, 185, 134, sw=9),
    ]
    return wrap([lens] + rays)


# ---------------------------------------------------------------------------
# Entry point
# ---------------------------------------------------------------------------

SYMBOLS = {
    "oil":       gen_oil,
    "bat":       gen_bat,
    "cel":       gen_cel,
    "hazard":    gen_hazard,
    "low_beam":  gen_low_beam,
    "high_beam": gen_high_beam,
}

if __name__ == "__main__":
    out_dir = os.path.join(os.path.dirname(__file__), "..", "assets", "warnings")
    os.makedirs(out_dir, exist_ok=True)

    for name, fn in SYMBOLS.items():
        svg  = fn()
        dest = os.path.join(out_dir, f"{name}.svg")
        with open(dest, "w") as f:
            f.write(svg)
        print(f"  wrote {dest}")

    print("Done.")
