# Player skin tones (draft)

Draft palettes that give the FRLG player (Red and Leaf) three skin tones: light (the original art), brown and dark. Nothing here is wired into the build yet.

- `make_skin_tones.py` regenerates everything in this folder from the sprites in `graphics/`. Run it from anywhere; it needs Pillow.
- `pal/` holds one JASC palette per gender, sprite and tone: `<red|leaf>_<overworld|front|back>_<light|brown|dark>.pal`.
- `leaf_front_hair_split.png` is Leaf's front picture with 56 hair pixels moved from palette slot 3 (shared with skin) to the unused slot 11. The `leaf_front_*.pal` palettes assume this version of the picture.
- `skin_tone_preview.png` shows every tone on the walking sheet, front picture and battle back sprite.

## Known issues

- Leaf's walking frames that face away from the camera use skin slot 3 for her hair, so the dark tone darkens her hair too. All 16 slots of her overworld palette are in use; fixing this means merging two near-identical colours to free a slot.
- The darkest skin shade (the outline) is shared with hair outlines on both characters, so hair outlines darken slightly in the dark tone.
- Only the normal walking sheet is previewed. The bike, surf, fishing and other sheets share its palette.
