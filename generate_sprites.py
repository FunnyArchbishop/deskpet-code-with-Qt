# DeskPet Qt - Sprite Image Generator
# Generates default PNG sprites for the desktop pet.
# Users can replace these images with their own character art.
# Usage: python generate_sprites.py

import os
from PIL import Image, ImageDraw

SPRITE_SIZE = 128  # sprite image size
ASSETS_DIR  = "assets"

# Color palette
C_BODY       = (255, 170, 50)
C_BODY_DARK  = (200, 110, 20)
C_EAR_INNER  = (255, 200, 180)
C_EYE        = (100, 220, 100)
C_PUPIL      = (20, 20, 20)
C_NOSE       = (255, 130, 130)
C_WHITE      = (255, 255, 255)
C_TRANS      = (0, 0, 0, 0)

os.makedirs(ASSETS_DIR, exist_ok=True)
os.makedirs(os.path.join(ASSETS_DIR, "triggers"), exist_ok=True)

def new_img():
    return Image.new("RGBA", (SPRITE_SIZE, SPRITE_SIZE), C_TRANS)

def circ(draw, cx, cy, r, color, width=0):
    x, y = int(cx*SPRITE_SIZE), int(cy*SPRITE_SIZE)
    rad = int(r*SPRITE_SIZE)
    if width == 0:
        draw.ellipse([x-rad, y-rad, x+rad, y+rad], fill=color)
    else:
        draw.ellipse([x-rad, y-rad, x+rad, y+rad],
                     fill=None, outline=color, width=max(1,int(width*SPRITE_SIZE)))

def ellip(draw, cx, cy, rx, ry, color):
    x, y = int(cx*SPRITE_SIZE), int(cy*SPRITE_SIZE)
    rxx, ryy = int(rx*SPRITE_SIZE), int(ry*SPRITE_SIZE)
    draw.ellipse([x-rxx, y-ryy, x+rxx, y+ryy], fill=color)

def tri(draw, pts_r, color):
    pts = [(int(p[0]*SPRITE_SIZE), int(p[1]*SPRITE_SIZE)) for p in pts_r]
    draw.polygon(pts, fill=color)

def cat_base(draw, sitting=False):
    # ears
    tri(draw, [(0.22,0.25),(0.05,0.35),(0.30,0.38)], C_BODY)
    tri(draw, [(0.78,0.25),(0.95,0.35),(0.70,0.38)], C_BODY)
    # inner ears
    tri(draw, [(0.25,0.28),(0.12,0.35),(0.29,0.38)], C_EAR_INNER)
    tri(draw, [(0.75,0.28),(0.88,0.35),(0.71,0.38)], C_EAR_INNER)
    # head
    circ(draw, 0.50, 0.42, 0.26, C_BODY)
    # body
    if sitting:
        ellip(draw, 0.50, 0.72, 0.22, 0.18, C_BODY)
    else:
        ellip(draw, 0.50, 0.72, 0.18, 0.22, C_BODY)
    # forehead stripes
    circ(draw, 0.43, 0.35, 0.04, C_BODY_DARK)
    circ(draw, 0.57, 0.35, 0.04, C_BODY_DARK)
    circ(draw, 0.50, 0.32, 0.03, C_BODY_DARK)
    # paws
    if sitting:
        ellip(draw, 0.38, 0.85, 0.06, 0.08, C_BODY)
        ellip(draw, 0.62, 0.85, 0.06, 0.08, C_BODY)
    else:
        ellip(draw, 0.35, 0.88, 0.05, 0.07, C_BODY)
        ellip(draw, 0.65, 0.88, 0.05, 0.07, C_BODY)

def cat_face(draw, blink=False):
    if blink:
        draw.ellipse([int(0.38*SPRITE_SIZE), int(0.43*SPRITE_SIZE),
                      int(0.48*SPRITE_SIZE), int(0.44*SPRITE_SIZE)], fill=C_PUPIL)
        draw.ellipse([int(0.52*SPRITE_SIZE), int(0.43*SPRITE_SIZE),
                      int(0.62*SPRITE_SIZE), int(0.44*SPRITE_SIZE)], fill=C_PUPIL)
    else:
        circ(draw, 0.43, 0.43, 0.06, C_EYE)
        circ(draw, 0.57, 0.43, 0.06, C_EYE)
        circ(draw, 0.43, 0.43, 0.028, C_PUPIL)
        circ(draw, 0.57, 0.43, 0.028, C_PUPIL)
        circ(draw, 0.41, 0.41, 0.014, C_WHITE)
        circ(draw, 0.55, 0.41, 0.014, C_WHITE)
    # nose
    ellip(draw, 0.50, 0.49, 0.03, 0.025, C_NOSE)
    # mouth
    w = max(1, int(0.02*SPRITE_SIZE))
    draw.arc([int(0.44*SPRITE_SIZE), int(0.50*SPRITE_SIZE),
              int(0.50*SPRITE_SIZE), int(0.54*SPRITE_SIZE)], 0, 180, fill=C_PUPIL, width=w)
    draw.arc([int(0.50*SPRITE_SIZE), int(0.50*SPRITE_SIZE),
              int(0.56*SPRITE_SIZE), int(0.54*SPRITE_SIZE)], 0, 180, fill=C_PUPIL, width=w)
    # whiskers
    for side, dx in [(-1, -0.08), (1, 0.08)]:
        cx = 0.50 + side*0.06
        for dy in [-0.02, 0.0, 0.02]:
            x1, y1 = int(cx*SPRITE_SIZE), int((0.49+dy)*SPRITE_SIZE)
            x2, y2 = int((cx+dx)*SPRITE_SIZE), int((0.49+dy+side*0.01)*SPRITE_SIZE)
            draw.line([x1,y1,x2,y2], fill=C_WHITE, width=max(1,int(0.01*SPRITE_SIZE)))

def cat_tail(draw, wag=0):
    base = (0.68, 0.80)
    mid  = (0.78, 0.62)
    tip  = (0.85, 0.55 + wag*0.1)
    pts = []
    for t in range(0, 101, 5):
        tt = t/100.0
        x = (1-tt)**2*base[0] + 2*(1-tt)*tt*mid[0] + tt**2*tip[0]
        y = (1-tt)**2*base[1] + 2*(1-tt)*tt*mid[1] + tt**2*tip[1]
        pts.append((int(x*SPRITE_SIZE), int(y*SPRITE_SIZE)))
    if len(pts) >= 2:
        draw.line(pts, fill=C_BODY, width=max(3,int(0.04*SPRITE_SIZE)), joint="curve")

def happy_fx(draw):
    import random
    rng = random.Random(42)
    hc = [(255,100,100), (255,150,150), (255,200,100)]
    for i in range(3):
        cx, cy = 0.15+rng.random()*0.70, 0.08+rng.random()*0.25
        c = hc[i]
        circ(draw, cx-0.02, cy, 0.025, c)
        circ(draw, cx+0.02, cy, 0.025, c)
        tri(draw, [(cx-0.04,cy+0.01),(cx+0.04,cy+0.01),(cx,cy+0.06)], c)
    sc = (255,255,100)
    for _ in range(5):
        cx, cy = 0.10+rng.random()*0.80, 0.08+rng.random()*0.30
        s = int((0.015+rng.random()*0.015)*SPRITE_SIZE)
        x, y = int(cx*SPRITE_SIZE), int(cy*SPRITE_SIZE)
        sw = max(1,int(s*0.4))
        draw.line([x-s,y,x+s,y], fill=sc, width=sw)
        draw.line([x,y-s,x,y+s], fill=sc, width=sw)

def save_sprite(name, fn, *args):
    img = new_img()
    d = ImageDraw.Draw(img)
    fn(d, *args)
    path = os.path.join(ASSETS_DIR, f"{name}.png")
    img.save(path)
    print(f"  OK {path}")

print("Generating sprite images...")

save_sprite("sprite_idle",  lambda d: (cat_base(d), cat_face(d), cat_tail(d)))
save_sprite("sprite_blink", lambda d: (cat_base(d), cat_face(d, blink=True), cat_tail(d)))
save_sprite("sprite_walk1", lambda d: (cat_base(d), cat_face(d), cat_tail(d, wag=-1)))
save_sprite("sprite_walk2", lambda d: (cat_base(d), cat_face(d), cat_tail(d, wag=1)))
save_sprite("sprite_sit",   lambda d: (cat_base(d, sitting=True), cat_face(d)))

save_sprite("triggers/happy",
    lambda d: (cat_base(d), cat_face(d), cat_tail(d, wag=2), happy_fx(d)))

print("Done! Images are in:", os.path.abspath(ASSETS_DIR))
print("You can replace these PNG files with your own character artwork.")
