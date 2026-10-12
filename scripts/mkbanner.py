#!/usr/bin/env python3
"""
Regenerate the store/README banner at resources/images/banner.png (720x320).

A dark banner: a stylised Pebble watch on the left showing the app's own
screen, amber "chime" arcs radiating from it, and the app name + tagline on
the right.

    pip install pillow
    python3 scripts/mkbanner.py
"""
import math

from PIL import Image, ImageDraw, ImageFont

W, H = 720, 320          # final size (Rebble store banner)
SS = 3                   # supersample factor
OUT = "resources/images/banner.png"

FONT_DIR = "/usr/share/fonts/TTF/"
F_THIN = FONT_DIR + "DejaVuSans-ExtraLight.ttf"
F_REG = FONT_DIR + "DejaVuSans.ttf"
F_BOLD = FONT_DIR + "DejaVuSans-Bold.ttf"

BG_TOP = (26, 26, 30)
BG_BOT = (39, 39, 45)
WHITE = (245, 245, 247)
GRAY = (150, 150, 158)
AMBER = (255, 176, 32)
BODY = (58, 58, 60)
BODY_EDGE = (78, 78, 82)
SCREEN = (11, 11, 12)

nw, nh = W * SS, H * SS
img = Image.new("RGB", (nw, nh), BG_TOP)
d = ImageDraw.Draw(img)

# --- background gradient -------------------------------------------------
for y in range(nh):
    t = y / (nh - 1)
    d.line([(0, y), (nw, y)],
           fill=tuple(round(a + (b - a) * t) for a, b in zip(BG_TOP, BG_BOT)))

# thin amber baseline
d.rectangle([0, nh - 3 * SS, nw, nh], fill=AMBER)


def font(path, px):
    return ImageFont.truetype(path, int(px * SS))


def fit(path, px, s, max_w, spacing=0):
    """Largest font <= px whose rendered width fits max_w (unscaled px)."""
    while px > 6:
        f = font(path, px)
        w = d.textlength(s, font=f) + spacing * SS * (len(s) - 1)
        if w <= max_w * SS:
            return f
        px -= 1
    return font(path, px)


def text(xy, s, fnt, fill, spacing=0, anchor="la"):
    """Draw text with optional per-character letter-spacing (px, unscaled)."""
    if not spacing:
        d.text((xy[0] * SS, xy[1] * SS), s, font=fnt, fill=fill, anchor=anchor)
        return
    x, y = xy[0] * SS, xy[1] * SS
    for ch in s:
        d.text((x, y), ch, font=fnt, fill=fill, anchor="l" + anchor[1])
        x += d.textlength(ch, font=fnt) + spacing * SS


# --- left: the watch ---------------------------------------------------
cx, cy = 150, 160            # watch centre
bw, bh, br = 140, 166, 28    # body w/h/corner-radius
bx0, by0 = cx - bw / 2, cy - bh / 2
d.rounded_rectangle([bx0 * SS, (by0 + 7) * SS, (bx0 + bw) * SS, (by0 + bh + 9) * SS],
                    radius=br * SS, fill=(14, 14, 16))  # drop shadow
d.rounded_rectangle([bx0 * SS, by0 * SS, (bx0 + bw) * SS, (by0 + bh) * SS],
                    radius=br * SS, fill=BODY, outline=BODY_EDGE, width=2 * SS)

pad = 13
sx0, sy0 = bx0 + pad, by0 + pad
sw, sh = bw - 2 * pad, bh - 2 * pad
d.rounded_rectangle([sx0 * SS, sy0 * SS, (sx0 + sw) * SS, (sy0 + sh) * SS],
                    radius=(br - pad) * SS, fill=SCREEN)

# on-screen: time + info, echoing src/c/main.c
text((cx, sy0 + 40), "09:00", font(F_THIN, 34), WHITE, anchor="mm")
d.line([(sx0 + 16) * SS, (sy0 + 66) * SS, (sx0 + sw - 16) * SS, (sy0 + 66) * SS],
       fill=(40, 40, 44), width=SS)
for i, line in enumerate(("Next 10:00", "sound + vibe", "active 08-22")):
    text((cx, sy0 + 84 + i * 20), line, font(F_REG, 12),
         GRAY if i else WHITE, anchor="mm")

# --- amber chime arcs radiating up/right ------------------------------
ox, oy = cx + bw / 2 - 14, cy - bh / 2 + 16
for i, r in enumerate((30, 48, 66)):
    box = [(ox - r) * SS, (oy - r) * SS, (ox + r) * SS, (oy + r) * SS]
    d.arc(box, start=-66, end=14, fill=AMBER, width=int((6 - i * 1.2) * SS))

# --- right: name + tagline -------------------------------------------
tx = 286
rmax = W - 24 - tx
text((tx, 98), "Hourly Chime", fit(F_BOLD, 44, "Hourly Chime", rmax), WHITE,
     anchor="lm")
text((tx, 146), "Chime on the hour — speaker, vibration, or both.",
     fit(F_REG, 17, "Chime on the hour — speaker, vibration, or both.", rmax),
     GRAY, anchor="lm")
text((tx, 188), "PEBBLE TIME 2    ·    PEBBLE 2 DUO",
     fit(F_BOLD, 14, "PEBBLE TIME 2    ·    PEBBLE 2 DUO", rmax, spacing=2),
     AMBER, spacing=2, anchor="lm")
text((tx, 224), "Westminster · Cuckoo · Mario · Tetris · and 8 more",
     fit(F_REG, 14, "Westminster · Cuckoo · Mario · Tetris · and 8 more", rmax),
     GRAY, anchor="lm")

# --------------------------------------------------------------------
img.resize((W, H), Image.LANCZOS).save(OUT)
print("wrote", OUT)
