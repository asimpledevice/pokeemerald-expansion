"""Draft main clothing colours for the FRLG player sprites and render a preview sheet.

Red's main colour is his jacket and hat; Leaf's is her hat.

Usage: python3 dev_scripts/player_clothing/make_clothing_colours.py [repo root]
Writes .pal files, re-indexed sprites and a preview next to this script. Needs Pillow.
"""
import glob
import os
import sys

from PIL import Image, ImageDraw

OUT = os.path.dirname(os.path.abspath(__file__))
REPO = sys.argv[1] if len(sys.argv) > 1 else os.path.join(OUT, "..", "..")


def gba(c):
    """Snap a colour to what the GBA can show (5 bits per channel)."""
    return tuple((v >> 3) << 3 for v in c)


# Each colour is a ramp of light, mid and dark. "red" and "white" are the original art.
COLOURS = {
    "red": [(255, 106, 74), (197, 57, 57), (106, 41, 41)],
    "white": [(238, 238, 255), (180, 180, 213), (106, 106, 139)],
    "blue": [(99, 148, 255), (49, 82, 205), (24, 41, 106)],
    "green": [(115, 213, 90), (57, 148, 57), (24, 74, 32)],
    "yellow": [(255, 222, 74), (222, 172, 32), (115, 82, 16)],
    "orange": [(255, 164, 57), (222, 115, 24), (115, 57, 16)],
    "pink": [(255, 156, 197), (222, 98, 148), (115, 41, 74)],
    "purple": [(189, 123, 238), (131, 74, 189), (65, 33, 106)],
    "black": [(99, 99, 115), (57, 57, 66), (24, 24, 32)],
}

# Per sprite: the main colour's palette slots (light, mid, dark or None), and how
# to pull non-clothing pixels out of those slots first:
#   None    - nothing to separate
#   "touch" - dark-slot pixels that touch no light or mid pixel are hair (Red's battle back)
#   "lower" - groups centred in the lower half of the frame are socks, shoes or a Poke Ball
#   "small" - groups of fewer than 4 pixels are an eye or a stray highlight
# Separated pixels keep their colour in one spare slot (merging the two closest
# colours if there's none). Leaf's walking sprite is left alone: at that size her
# socks matching her hat reads as an outfit, like Red's shoes matching his jacket.
SPRITES = {
    "red": {
        "overworld": ("graphics/object_events/pics/people/red/*.png", (11, 12, 8), None),
        "front": ("graphics/trainers/front_pics/red.png", (12, 13, None), None),
        "back": ("graphics/trainers/back_pics/red.png", (11, 12, 8), "touch"),
    },
    "leaf": {
        "overworld": ("graphics/object_events/pics/people/leaf/*.png", (9, 10, None), None),
        # The skin-tone script's version, where hair no longer shares skin slot 3.
        "front": ("dev_scripts/player_skin_tones/leaf_front_hair_split.png", (14, 9, None), "lower"),
        "back": ("graphics/trainers/back_pics/leaf.png", (9, 10, None), "small"),
    },
}

# Frame height for each sprite kind, so pixels are judged frame by frame.
FRAME_H = {"overworld": 32, "front": 64, "back": 64}


def components(px, w, y0, y1, wanted):
    """8-connected groups of pixels whose slot is in `wanted`, inside rows y0..y1."""
    seen, groups = set(), []
    for y in range(y0, y1):
        for x in range(w):
            if (x, y) in seen or px[x, y] not in wanted:
                continue
            stack, group = [(x, y)], []
            seen.add((x, y))
            while stack:
                cx, cy = stack.pop()
                group.append((cx, cy))
                for nx in range(cx - 1, cx + 2):
                    for ny in range(cy - 1, cy + 2):
                        if 0 <= nx < w and y0 <= ny < y1 and (nx, ny) not in seen and px[nx, ny] in wanted:
                            seen.add((nx, ny))
                            stack.append((nx, ny))
            groups.append(group)
    return groups


def pixels_to_separate(im, kind, slots, mode):
    px = im.load()
    w, h = im.size
    fh = FRAME_H[kind]
    main = {s for s in slots if s is not None}
    found = []
    for y0 in range(0, h, fh):
        if mode == "touch":
            light_mid = {slots[0], slots[1]}
            for y in range(y0, y0 + fh):
                for x in range(w):
                    if px[x, y] == slots[2] and not any(
                            px[nx, ny] in light_mid
                            for ny in range(max(y0, y - 1), min(y0 + fh, y + 2))
                            for nx in range(max(0, x - 1), min(w, x + 2))):
                        found.append((x, y))
        else:
            for group in components(px, w, y0, y0 + fh, main):
                cy = sum(y for _, y in group) / len(group) - y0
                if (mode == "lower" and cy >= fh * 0.5) or (mode == "small" and len(group) < 4):
                    found += group
    return found


def spare_slot(im, keep):
    """An unused slot, or one freed by merging the two closest colours not in `keep`."""
    used = set(im.getdata())
    for s in range(1, 16):
        if s not in used:
            return s, None
    pal = im.getpalette()
    cands = [s for s in range(1, 16) if s not in keep]
    best = min(((sum((p - q) ** 2 for p, q in zip(pal[a * 3:a * 3 + 3], pal[b * 3:b * 3 + 3])), a, b)
                for i, a in enumerate(cands) for b in cands[i + 1:]))
    _, into, drop = best
    px = im.load()
    for y in range(im.size[1]):
        for x in range(im.size[0]):
            if px[x, y] == drop:
                px[x, y] = into
    note = "merged slot %d %s into slot %d %s" % (
        drop, tuple(pal[drop * 3:drop * 3 + 3]), into, tuple(pal[into * 3:into * 3 + 3]))
    return drop, note


def separate(im, kind, slots, mode, notes, label):
    if mode is None:
        return im
    im = im.copy()
    found = pixels_to_separate(im, kind, slots, mode)
    if not found:
        return im
    px = im.load()
    # Use the lightest separated colour for the spare slot.
    src = min({px[x, y] for x, y in found}, key=lambda s: slots.index(s))
    # Skin slots and the main colour's slots are never merged away.
    keep = {0, 1, 2, 3, 4, 11} | {s for s in slots if s is not None}
    dst, note = spare_slot(im, keep)
    if note:
        notes.append("%s: %s" % (label, note))
    pal = im.getpalette()
    pal[dst * 3:dst * 3 + 3] = pal[src * 3:src * 3 + 3]
    im.putpalette(pal)
    for x, y in found:
        px[x, y] = dst
    notes.append("%s: moved %d non-clothing pixels into slot %d" % (label, len(found), dst))
    return im


def recolour(im, slots, ramp):
    im = im.copy()
    pal = im.getpalette()
    for slot, colour in zip(slots, ramp):
        if slot is not None:
            pal[slot * 3:slot * 3 + 3] = gba(colour)
    im.putpalette(pal)
    return im


def write_pal(path, im):
    pal = im.getpalette()[:48]
    with open(path, "w", newline="\r\n") as f:
        f.write("JASC-PAL\n0100\n16\n")
        for i in range(16):
            f.write("%d %d %d\n" % tuple(pal[i * 3:i * 3 + 3]))


def main():
    notes = []
    prepared = {}
    os.makedirs(os.path.join(OUT, "sprites"), exist_ok=True)
    for who, kinds in SPRITES.items():
        for kind, (pattern, slots, mode) in kinds.items():
            path = sorted(glob.glob(os.path.join(REPO, pattern)))
            path = next((p for p in path if p.endswith("_normal.png")), path[0])
            im = separate(Image.open(path), kind, slots, mode, notes, "%s %s" % (who, kind))
            if mode is not None:
                im.save(os.path.join(OUT, "sprites", "%s_%s_separated.png" % (who, kind)))
            prepared[(who, kind)] = (im, slots)

    os.makedirs(os.path.join(OUT, "pal"), exist_ok=True)
    order = ["original"] + [c for c in COLOURS if c not in ("red", "white")]
    rows = []
    for colour in order:
        row = []
        for who in SPRITES:
            name = ("red" if who == "red" else "white") if colour == "original" else colour
            for kind in ("overworld", "front", "back"):
                im, slots = prepared[(who, kind)]
                r = recolour(im, slots, COLOURS[name])
                write_pal(os.path.join(OUT, "pal", "%s_%s_%s.pal" % (who, kind, name)), r)
                row.append(r.convert("RGB"))
        rows.append((colour, row))

    # Preview: a row per colour; Red then Leaf, each as walk frames 0-2, front, back.
    s = 2
    cell_ow, cell_pic = 48 * 3, 64 * s
    W = 80 + 2 * (cell_ow + 8 + cell_pic + 8 + cell_pic + 24)
    H = 24 + len(rows) * (cell_pic + 8)
    sheet = Image.new("RGB", (W, H), (255, 255, 255))
    d = ImageDraw.Draw(sheet)
    for gi, who in enumerate(("Red", "Leaf")):
        d.text((80 + gi * (cell_ow + cell_pic * 2 + 40), 6), who, fill=(0, 0, 0))
    for r, (colour, row) in enumerate(rows):
        y = 24 + r * (cell_pic + 8)
        d.text((8, y + 8), colour, fill=(0, 0, 0))
        x = 80
        for gi in range(2):
            ow, fr, bk = row[gi * 3:gi * 3 + 3]
            sheet.paste(ow.crop((0, 0, 48, 32)).resize((cell_ow, 32 * 3), Image.NEAREST), (x, y + 16))
            x += cell_ow + 8
            sheet.paste(fr.resize((cell_pic, cell_pic), Image.NEAREST), (x, y))
            x += cell_pic + 8
            sheet.paste(bk.crop((0, 0, 64, 64)).resize((cell_pic, cell_pic), Image.NEAREST), (x, y))
            x += cell_pic + 24
    sheet.save(os.path.join(OUT, "clothing_preview.png"))
    print("\n".join(notes))
    print("wrote", os.path.join(OUT, "clothing_preview.png"))


main()
