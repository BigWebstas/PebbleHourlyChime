#!/usr/bin/env python3
"""
Regenerate the launcher icon at resources/images/menu_icon.png.

A 25x25 (the Pebble menu-icon maximum) white-on-transparent clock face:
circular rim, 12/3/6/9 ticks, minute hand up, hour hand toward 4.

    pip install pillow
    python3 scripts/mkicon.py
"""
import math

from PIL import Image, ImageDraw

S = 25          # final size (Pebble menu-icon max is 25x25)
SS = 20         # supersample factor
N = S * SS
OUT = "resources/images/menu_icon.png"

img = Image.new("L", (N, N), 0)
d = ImageDraw.Draw(img)

cx = cy = N / 2 - 0.5
R = N * 0.47
ring = N * 0.10

# rim
d.ellipse([cx - R, cy - R, cx + R, cy + R], outline=255, width=int(ring))

# hour ticks at 12 / 3 / 6 / 9
tick_len = N * 0.085
tick_w = int(N * 0.06)
for ang in (0, 90, 180, 270):
    a = math.radians(ang)
    r1 = R - ring
    r2 = r1 - tick_len
    d.line([cx + r1 * math.sin(a), cy - r1 * math.cos(a),
            cx + r2 * math.sin(a), cy - r2 * math.cos(a)], fill=255, width=tick_w)


def hand(angle_deg, length, width):
    a = math.radians(angle_deg)
    d.line([cx, cy, cx + length * math.sin(a), cy - length * math.cos(a)],
           fill=255, width=int(width))


hand(0, R * 0.60, N * 0.06)      # minute hand: 12
hand(120, R * 0.42, N * 0.075)   # hour hand: ~4
d.ellipse([cx - N * 0.045, cy - N * 0.045, cx + N * 0.045, cy + N * 0.045], fill=255)

small = img.resize((S, S), Image.LANCZOS)
mask = small.point(lambda p: 255 if p >= 100 else 0)
out = Image.composite(Image.new("RGBA", (S, S), (255, 255, 255, 255)),
                      Image.new("RGBA", (S, S), (0, 0, 0, 0)), mask)
out.save(OUT)
print("wrote", OUT)
