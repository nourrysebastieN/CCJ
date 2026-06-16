# CCJ Dashboard Assets

Place PNG images here. All images should match the gauge bounds aspect ratio.

## Required files

| File              | Content                                                       |
|-------------------|---------------------------------------------------------------|
| `dashboard_bg.png`| Full-window dashboard background (1280×480 recommended)       |
| `rpm_bg.png`      | Tachometer face (scale markings, redline zone, labels)        |
| `speed_bg.png`    | Speedometer face                                              |
| `fuel_bg.png`     | Fuel gauge face                                               |
| `temp_bg.png`     | Coolant temperature gauge face                                |
| `needle.png`      | Needle sprite — pointing UP (12 o'clock), centered on canvas |

## Needle image convention

The needle image must be drawn pointing **straight up** at the center of the canvas.
`SDL_RenderCopyEx` rotates it clockwise from there:

- **0 / min value** → 225° clockwise from up (≈ 7-8 o'clock, bottom-left)
- **max value**     → 495° / 135° clockwise from up (≈ 4-5 o'clock, bottom-right)

The rotation pivot is the center of the gauge bounds, so the needle's axis of rotation
should also be centered in the image canvas.

## Fallback

If any file is missing, the affected gauge renders a stub (dark background + red line needle).
The dashboard runs without any assets present.
