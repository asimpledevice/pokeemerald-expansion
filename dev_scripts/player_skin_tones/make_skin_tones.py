"""Draft skin-tone palettes for the FRLG player sprites and render preview sheets.

Usage: python3 dev_scripts/player_skin_tones/make_skin_tones.py [repo root]
Writes .pal files and previews next to this script. Needs Pillow.
"""
import os
import sys

from PIL import Image, ImageDraw

OUT = os.path.dirname(os.path.abspath(__file__))
REPO = sys.argv[1] if len(sys.argv) > 1 else os.path.join(OUT, "..", "..")


def gba(c):
    """Snap a colour to what the GBA can show (5 bits per channel)."""
    return tuple((v >> 3) << 3 for v in c)


# Skin ramps, lightest to darkest. "light" is the original FRLG art.
# Overworld and back sprites use 3 skin slots: highlight, base, outline.
OW_RAMPS = {
    "light": [(255, 197, 148), (222, 148, 115), (123, 65, 65)],
    "brown": [(222, 164, 115), (180, 115, 74), (106, 57, 41)],
    "dark": [(164, 106, 74), (115, 74, 49), (65, 41, 32)],
}
OW_SLOTS = [2, 3, 4]

# Front pictures use 4 skin slots: highlight, light, shadow, outline.
FRONT_RAMPS = {
    "light": [(255, 230, 180), (230, 189, 156), (205, 148, 115), (123, 90, 82)],
    "brown": [(230, 189, 140), (197, 148, 106), (164, 115, 74), (106, 66, 49)],
    "dark": [(172, 123, 90), (139, 98, 66), (106, 74, 49), (65, 41, 32)],
}
FRONT_SLOTS = [1, 2, 3, 4]

# Slots that also colour hair. Pixels of these slots that are hair, not skin,
# get moved to a spare slot holding the original colour, so the tone swap
# leaves the hair alone.
HAIR_SPLIT = {
    # file: [(shared slot, spare slot)]
    "front_pics/leaf.png": [(3, 11)],
}

SPRITES = {
    "red": {
        "ow": "graphics/object_events/pics/people/red/red_normal.png",
        "front": "graphics/trainers/front_pics/red.png",
        "back": "graphics/trainers/back_pics/red.png",
    },
    "leaf": {
        "ow": "graphics/object_events/pics/people/leaf/green_normal.png",
        "front": "graphics/trainers/front_pics/leaf.png",
        "back": "graphics/trainers/back_pics/leaf.png",
    },
}


def split_hair(im, shared, spare):
    """Move hair pixels of `shared` to `spare`.

    A pixel counts as skin if a lighter skin slot (1 or 2) is within 2 pixels;
    hair highlights sit among hair browns instead.
    """
    im = im.copy()
    px = im.load()
    w, h = im.size
    pal = im.getpalette()
    pal[spare * 3:spare * 3 + 3] = pal[shared * 3:shared * 3 + 3]
    im.putpalette(pal)
    moved = []
    for y in range(h):
        for x in range(w):
            if px[x, y] != shared:
                continue
            near_skin = any(
                px[nx, ny] in (1, 2)
                for ny in range(max(0, y - 2), min(h, y + 3))
                for nx in range(max(0, x - 2), min(w, x + 3))
            )
            if not near_skin:
                moved.append((x, y))
    for x, y in moved:
        px[x, y] = spare
    return im, len(moved)


def recolour(im, slots, ramp):
    im = im.copy()
    pal = im.getpalette()
    for slot, colour in zip(slots, ramp):
        pal[slot * 3:slot * 3 + 3] = gba(colour)
    im.putpalette(pal)
    return im


def write_pal(path, im):
    pal = im.getpalette()[:48]
    with open(path, "w", newline="\r\n") as f:
        f.write("JASC-PAL\n0100\n16\n")
        for i in range(16):
            f.write("%d %d %d\n" % tuple(pal[i * 3:i * 3 + 3]))


def load(rel):
    im = Image.open(os.path.join(REPO, rel))
    key = rel.split("graphics/trainers/")[-1]
    report = []
    for shared, spare in HAIR_SPLIT.get(key, []):
        im, n = split_hair(im, shared, spare)
        report.append("%s: moved %d hair pixels from slot %d to slot %d" % (key, n, shared, spare))
    return im, report


def main():
    os.makedirs(os.path.join(OUT, "pal"), exist_ok=True)
    notes = []
    tiles = {}
    for who, files in SPRITES.items():
        ow, r1 = load(files["ow"])
        front, r2 = load(files["front"])
        back, r3 = load(files["back"])
        notes += r1 + r2 + r3
        if r2:
            front.save(os.path.join(OUT, "%s_front_hair_split.png" % who))
        for tone in OW_RAMPS:
            o = recolour(ow, OW_SLOTS, OW_RAMPS[tone])
            fr = recolour(front, FRONT_SLOTS, FRONT_RAMPS[tone])
            b = recolour(back, OW_SLOTS, OW_RAMPS[tone])
            write_pal(os.path.join(OUT, "pal", "%s_overworld_%s.pal" % (who, tone)), o)
            write_pal(os.path.join(OUT, "pal", "%s_front_%s.pal" % (who, tone)), fr)
            write_pal(os.path.join(OUT, "pal", "%s_back_%s.pal" % (who, tone)), b)
            tiles[(who, tone)] = (o.convert("RGB"), fr.convert("RGB"), b.crop((0, 0, 64, 64)).convert("RGB"))

    # Preview: a row per gender and tone; overworld walk frames, front pic, back pic.
    s_ow, s_pic = 4, 2
    ow_w, ow_h = 144 * s_ow, 32 * s_ow
    pic = 64 * s_pic
    row_h = max(ow_h, pic) + 16
    width = 90 + ow_w + 16 + pic + 16 + pic + 16
    sheet = Image.new("RGB", (width, 24 + row_h * len(tiles)), (255, 255, 255))
    d = ImageDraw.Draw(sheet)
    d.text((90, 6), "overworld (walk frames)", fill=(0, 0, 0))
    d.text((90 + ow_w + 16, 6), "front picture", fill=(0, 0, 0))
    d.text((90 + ow_w + 16 + pic + 16, 6), "battle back", fill=(0, 0, 0))
    for r, ((who, tone), (o, fr, b)) in enumerate(tiles.items()):
        y = 24 + r * row_h
        d.text((8, y + 8), "%s / %s" % (who, tone), fill=(0, 0, 0))
        sheet.paste(o.resize((ow_w, ow_h), Image.NEAREST), (90, y))
        sheet.paste(fr.resize((pic, pic), Image.NEAREST), (90 + ow_w + 16, y))
        sheet.paste(b.resize((pic, pic), Image.NEAREST), (90 + ow_w + 16 + pic + 16, y))
    sheet.save(os.path.join(OUT, "skin_tone_preview.png"))
    print("\n".join(notes))
    print("wrote", os.path.join(OUT, "skin_tone_preview.png"))


main()
