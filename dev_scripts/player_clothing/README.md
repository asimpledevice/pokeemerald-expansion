# Player hat colours (draft)

Draft palettes that let the player pick a hat colour, for both Red and Leaf. Colours: the original (red for Red, white for Leaf), blue, green, yellow, orange, pink, purple and black. Only the hat changes; Red's jacket stays red so it stays distinct from his backpack. Nothing here is wired into the build yet.

- `make_clothing_colours.py` regenerates everything in this folder from the sprites in `graphics/` (and Leaf's hair-split front picture from `../player_skin_tones/`). Run it from anywhere; it needs Pillow.
- `pal/` holds one JASC palette per character, sprite and colour: `<red|leaf>_<overworld|front|back>_<colour>.pal`.
- `sprites/` holds the pictures whose pixels had to move so that only the hat uses the hat's palette slots. The palettes for those sprites assume these versions.
- `clothing_preview.png` shows every colour on the first walking frames, the front picture and the battle back sprite.

## What moved

Red's hat and jacket share their red slots, and his palettes have no free slots, so the jacket's two main shades moved to slots freed by merging his least-used near-duplicate colours. Merges never go into skin, hair or hat slots, since those change with the wardrobe.

| Sprite | Moved | Merged to make room |
| --- | --- | --- |
| Red walking (all 7 sheets) | Jacket light and mid shades | Slot 1 into slot 14 (two browns, 128 px); slot 5 into slot 6 (two jeans blues, 45 px) |
| Red front | Jacket, wristband and shoe reds | Slot 10 into slot 9 (5 px); slot 14 into slot 9 (pure white into near-white, 22 px) |
| Red battle back | Hair under the hat (283 px, into unused slot 1); jacket collar reds | Slot 7 into slot 10 (3 px); slot 5 into slot 6 (two blues, 49 px) |
| Leaf front | Socks, shoes and Poke Ball whites, together in one slot | Slot 7 into slot 6 (two blues on her top, 37 px) |
| Leaf battle back | A few stray white pixels near her eye (into unused slot 5) | None |

## Notes

- Red's hat and jacket still share their thin dark outline (one slot), so the jacket's outline takes on a dark shade of the hat colour. There's no spare slot to separate it.
- On Leaf's walking sprite her socks change colour with her hat; at that size it reads as an outfit.
- In the game, skin tone and hat colour would be applied together at load time by writing both ramps into the base palette, rather than storing a palette for every combination.
